/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilter.h"

#include "SkBitmap.h"
#include "SkChecksum.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRect.h"
#include "SkTDynamicHash.h"
#include "SkValidationUtils.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkGrPixelRef.h"
#include "SkGr.h"
#endif

SkImageFilter::Common::~Common() {
    for (int i = 0; i < fInputs.count(); ++i) {
        SkSafeUnref(fInputs[i]);
    }
}

void SkImageFilter::Common::allocInputs(int count) {
    const size_t size = count * sizeof(SkImageFilter*);
    fInputs.reset(count);
    sk_bzero(fInputs.get(), size);
}

void SkImageFilter::Common::detachInputs(SkImageFilter** inputs) {
    const size_t size = fInputs.count() * sizeof(SkImageFilter*);
    memcpy(inputs, fInputs.get(), size);
    sk_bzero(fInputs.get(), size);
}

bool SkImageFilter::Common::unflatten(SkReadBuffer& buffer, int expectedCount) {
    int count = buffer.readInt();
    if (expectedCount < 0) {    // means the caller doesn't care how many
        expectedCount = count;
    }
    if (!buffer.validate((count == expectedCount) && (count >= 0))) {
        return false;
    }

    this->allocInputs(count);
    for (int i = 0; i < count; i++) {
        if (buffer.readBool()) {
            fInputs[i] = buffer.readImageFilter();
        }
        if (!buffer.isValid()) {
            return false;
        }
    }
    SkRect rect;
    buffer.readRect(&rect);
    if (!buffer.isValid() || !buffer.validate(SkIsValidRect(rect))) {
        return false;
    }
    
    uint32_t flags = buffer.readUInt();
    fCropRect = CropRect(rect, flags);
    return buffer.isValid();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImageFilter::Cache* gExternalCache;

SkImageFilter::SkImageFilter(int inputCount, SkImageFilter** inputs, const CropRect* cropRect)
  : fInputCount(inputCount),
    fInputs(new SkImageFilter*[inputCount]),
    fCropRect(cropRect ? *cropRect : CropRect(SkRect(), 0x0)) {
    for (int i = 0; i < inputCount; ++i) {
        fInputs[i] = inputs[i];
        SkSafeRef(fInputs[i]);
    }
}

SkImageFilter::~SkImageFilter() {
    for (int i = 0; i < fInputCount; i++) {
        SkSafeUnref(fInputs[i]);
    }
    delete[] fInputs;
}

SkImageFilter::SkImageFilter(int inputCount, SkReadBuffer& buffer) {
    Common common;
    if (common.unflatten(buffer, inputCount)) {
        fCropRect = common.cropRect();
        fInputCount = common.inputCount();
        fInputs = SkNEW_ARRAY(SkImageFilter*, fInputCount);
        common.detachInputs(fInputs);
    } else {
        fInputCount = 0;
        fInputs = NULL;
    }
}

void SkImageFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fInputCount);
    for (int i = 0; i < fInputCount; i++) {
        SkImageFilter* input = getInput(i);
        buffer.writeBool(input != NULL);
        if (input != NULL) {
            buffer.writeFlattenable(input);
        }
    }
    buffer.writeRect(fCropRect.rect());
    buffer.writeUInt(fCropRect.flags());
}

bool SkImageFilter::filterImage(Proxy* proxy, const SkBitmap& src,
                                const Context& context,
                                SkBitmap* result, SkIPoint* offset) const {
    Cache* cache = context.cache();
    SkASSERT(result);
    SkASSERT(offset);
    SkASSERT(cache);
    if (cache->get(this, result, offset)) {
        return true;
    }
    /*
     *  Give the proxy first shot at the filter. If it returns false, ask
     *  the filter to do it.
     */
    if ((proxy && proxy->filterImage(this, src, context, result, offset)) ||
        this->onFilterImage(proxy, src, context, result, offset)) {
        cache->set(this, *result, *offset);
        return true;
    }
    return false;
}

bool SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                 SkIRect* dst) const {
    SkASSERT(&src);
    SkASSERT(dst);
    if (SkImageFilter::GetExternalCache()) {
        /*
         *  When the external cache is active, do not intersect the saveLayer
         *  bounds with the clip bounds. This is so that the cached result
         *  is always the full size of the primitive's bounds,
         *  regardless of the clip active on first draw.
         */
        *dst = SkIRect::MakeLargest();
        return true;
    }
    return this->onFilterBounds(src, ctm, dst);
}

void SkImageFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
    if (0 == fInputCount) {
        *dst = src;
        return;
    }
    if (this->getInput(0)) {
        this->getInput(0)->computeFastBounds(src, dst);
    } else {
        *dst = src;
    }
    for (int i = 1; i < fInputCount; i++) {
        SkImageFilter* input = this->getInput(i);
        if (input) {
            SkRect bounds;
            input->computeFastBounds(src, &bounds);
            dst->join(bounds);
        } else {
            dst->join(src);
        }
    }
}

bool SkImageFilter::onFilterImage(Proxy*, const SkBitmap&, const Context&,
                                  SkBitmap*, SkIPoint*) const {
    return false;
}

bool SkImageFilter::canFilterImageGPU() const {
    return this->asNewEffect(NULL, NULL, SkMatrix::I(), SkIRect());
}

bool SkImageFilter::filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                   SkBitmap* result, SkIPoint* offset) const {
#if SK_SUPPORT_GPU
    SkBitmap input = src;
    SkASSERT(fInputCount == 1);
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (this->getInput(0) &&
        !this->getInput(0)->getInputResultGPU(proxy, src, ctx, &input, &srcOffset)) {
        return false;
    }
    GrTexture* srcTexture = input.getTexture();
    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, input, &srcOffset, &bounds, &input)) {
        return false;
    }
    SkRect srcRect = SkRect::Make(bounds);
    SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
    GrContext* context = srcTexture->getContext();

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit,
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    GrAutoScratchTexture dst(context, desc);
    GrContext::AutoMatrix am;
    am.setIdentity(context);
    GrContext::AutoRenderTarget art(context, dst.texture()->asRenderTarget());
    GrContext::AutoClip acs(context, dstRect);
    GrEffect* effect;
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    bounds.offset(-srcOffset);
    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    this->asNewEffect(&effect, srcTexture, matrix, bounds);
    SkASSERT(effect);
    GrPaint paint;
    paint.addColorEffect(effect)->unref();
    context->drawRectToRect(paint, dstRect, srcRect);

    SkAutoTUnref<GrTexture> resultTex(dst.detach());
    WrapTexture(resultTex, bounds.width(), bounds.height(), result);
    return true;
#else
    return false;
#endif
}

bool SkImageFilter::applyCropRect(const Context& ctx, const SkBitmap& src,
                                  const SkIPoint& srcOffset, SkIRect* bounds) const {
    SkIRect srcBounds;
    src.getBounds(&srcBounds);
    srcBounds.offset(srcOffset);
    SkRect cropRect;
    ctx.ctm().mapRect(&cropRect, fCropRect.rect());
    SkIRect cropRectI;
    cropRect.roundOut(&cropRectI);
    uint32_t flags = fCropRect.flags();
    if (flags & CropRect::kHasLeft_CropEdge) srcBounds.fLeft = cropRectI.fLeft;
    if (flags & CropRect::kHasTop_CropEdge) srcBounds.fTop = cropRectI.fTop;
    if (flags & CropRect::kHasRight_CropEdge) srcBounds.fRight = cropRectI.fRight;
    if (flags & CropRect::kHasBottom_CropEdge) srcBounds.fBottom = cropRectI.fBottom;
    if (!srcBounds.intersect(ctx.clipBounds())) {
        return false;
    }
    *bounds = srcBounds;
    return true;
}

bool SkImageFilter::applyCropRect(const Context& ctx, Proxy* proxy, const SkBitmap& src,
                                  SkIPoint* srcOffset, SkIRect* bounds, SkBitmap* dst) const {
    SkIRect srcBounds;
    src.getBounds(&srcBounds);
    srcBounds.offset(*srcOffset);
    SkRect cropRect;
    ctx.ctm().mapRect(&cropRect, fCropRect.rect());
    SkIRect cropRectI;
    cropRect.roundOut(&cropRectI);
    uint32_t flags = fCropRect.flags();
    *bounds = srcBounds;
    if (flags & CropRect::kHasLeft_CropEdge) bounds->fLeft = cropRectI.fLeft;
    if (flags & CropRect::kHasTop_CropEdge) bounds->fTop = cropRectI.fTop;
    if (flags & CropRect::kHasRight_CropEdge) bounds->fRight = cropRectI.fRight;
    if (flags & CropRect::kHasBottom_CropEdge) bounds->fBottom = cropRectI.fBottom;
    if (!bounds->intersect(ctx.clipBounds())) {
        return false;
    }
    if (srcBounds.contains(*bounds)) {
        *dst = src;
        return true;
    } else {
        SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds->width(), bounds->height()));
        if (!device) {
            return false;
        }
        SkCanvas canvas(device);
        canvas.clear(0x00000000);
        canvas.drawBitmap(src, srcOffset->x() - bounds->x(), srcOffset->y() - bounds->y());
        *srcOffset = SkIPoint::Make(bounds->x(), bounds->y());
        *dst = device->accessBitmap(false);
        return true;
    }
}

bool SkImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   SkIRect* dst) const {
    if (fInputCount < 1) {
        return false;
    }

    SkIRect bounds;
    for (int i = 0; i < fInputCount; ++i) {
        SkImageFilter* filter = this->getInput(i);
        SkIRect rect = src;
        if (filter && !filter->filterBounds(src, ctm, &rect)) {
            return false;
        }
        if (0 == i) {
            bounds = rect;
        } else {
            bounds.join(rect);
        }
    }

    // don't modify dst until now, so we don't accidentally change it in the
    // loop, but then return false on the next filter.
    *dst = bounds;
    return true;
}

bool SkImageFilter::asNewEffect(GrEffect**, GrTexture*, const SkMatrix&, const SkIRect&) const {
    return false;
}

bool SkImageFilter::asColorFilter(SkColorFilter**) const {
    return false;
}

void SkImageFilter::SetExternalCache(Cache* cache) {
    SkRefCnt_SafeAssign(gExternalCache, cache);
}

SkImageFilter::Cache* SkImageFilter::GetExternalCache() {
    return gExternalCache;
}

#if SK_SUPPORT_GPU

void SkImageFilter::WrapTexture(GrTexture* texture, int width, int height, SkBitmap* result) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    result->setInfo(info);
    result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, texture)))->unref();
}

bool SkImageFilter::getInputResultGPU(SkImageFilter::Proxy* proxy,
                                      const SkBitmap& src, const Context& ctx,
                                      SkBitmap* result, SkIPoint* offset) const {
    // Ensure that GrContext calls under filterImage and filterImageGPU below will see an identity
    // matrix with no clip and that the matrix, clip, and render target set before this function was
    // called are restored before we return to the caller.
    GrContext* context = src.getTexture()->getContext();
    GrContext::AutoWideOpenIdentityDraw awoid(context, NULL);
    if (this->canFilterImageGPU()) {
        return this->filterImageGPU(proxy, src, ctx, result, offset);
    } else {
        if (this->filterImage(proxy, src, ctx, result, offset)) {
            if (!result->getTexture()) {
                const SkImageInfo info = result->info();
                if (kUnknown_SkColorType == info.colorType()) {
                    return false;
                }
                GrTexture* resultTex = GrLockAndRefCachedBitmapTexture(context, *result, NULL);
                result->setPixelRef(new SkGrPixelRef(info, resultTex))->unref();
                GrUnlockAndUnrefCachedBitmapTexture(resultTex);
            }
            return true;
        } else {
            return false;
        }
    }
}
#endif

class CacheImpl : public SkImageFilter::Cache {
public:
    explicit CacheImpl(int minChildren) : fMinChildren(minChildren) {
        SkASSERT(fMinChildren <= 2);
    }

    virtual ~CacheImpl();
    bool get(const SkImageFilter* key, SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
    void set(const SkImageFilter* key, const SkBitmap& result, const SkIPoint& offset) SK_OVERRIDE;
    void remove(const SkImageFilter* key) SK_OVERRIDE;
private:
    typedef const SkImageFilter* Key;
    struct Value {
        Value(Key key, const SkBitmap& bitmap, const SkIPoint& offset)
            : fKey(key), fBitmap(bitmap), fOffset(offset) {}
        Key fKey;
        SkBitmap fBitmap;
        SkIPoint fOffset;
        static const Key& GetKey(const Value& v) {
            return v.fKey;
        }
        static uint32_t Hash(Key key) {
            return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
        }
    };
    SkTDynamicHash<Value, Key> fData;
    int fMinChildren;
};

bool CacheImpl::get(const SkImageFilter* key, SkBitmap* result, SkIPoint* offset) {
    Value* v = fData.find(key);
    if (v) {
        *result = v->fBitmap;
        *offset = v->fOffset;
        return true;
    }
    return false;
}

void CacheImpl::remove(const SkImageFilter* key) {
    Value* v = fData.find(key);
    if (v) {
        fData.remove(key);
        delete v;
    }
}

void CacheImpl::set(const SkImageFilter* key, const SkBitmap& result, const SkIPoint& offset) {
    if (fMinChildren < 2 || !key->unique()) {
        // We take !key->unique() as a signal that there are probably at least 2 refs on the key,
        // meaning this filter probably has at least two children and is worth caching when
        // fMinChildren is 2.  If fMinChildren is less than two, we'll just always cache.
        fData.add(new Value(key, result, offset));
    }
}

SkImageFilter::Cache* SkImageFilter::Cache::Create(int minChildren) {
    return new CacheImpl(minChildren);
}

CacheImpl::~CacheImpl() {
    SkTDynamicHash<Value, Key>::Iter iter(&fData);

    while (!iter.done()) {
        Value* v = &*iter;
        ++iter;
        delete v;
    }
}
