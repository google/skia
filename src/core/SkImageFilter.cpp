/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilter.h"

#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkChecksum.h"
#include "SkDevice.h"
#include "SkLazyPtr.h"
#include "SkMatrixImageFilter.h"
#include "SkMutex.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkTDynamicHash.h"
#include "SkTInternalLList.h"
#include "SkValidationUtils.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "SkGrPixelRef.h"
#include "SkGr.h"
#endif

#ifdef SK_BUILD_FOR_IOS
  enum { kDefaultCacheSize = 2 * 1024 * 1024 };
#else
  enum { kDefaultCacheSize = 128 * 1024 * 1024 };
#endif

#ifndef SK_IGNORE_TO_STRING
void SkImageFilter::CropRect::toString(SkString* str) const {
    if (!fFlags) {
        return;
    }

    str->appendf("cropRect (");
    if (fFlags & CropRect::kHasLeft_CropEdge) {
        str->appendf("%.2f, ", fRect.fLeft);
    } else {
        str->appendf("X, ");
    }
    if (fFlags & CropRect::kHasTop_CropEdge) {
        str->appendf("%.2f, ", fRect.fTop);
    } else {
        str->appendf("X, ");
    }
    if (fFlags & CropRect::kHasWidth_CropEdge) {
        str->appendf("%.2f, ", fRect.width());
    } else {
        str->appendf("X, ");
    }
    if (fFlags & CropRect::kHasHeight_CropEdge) {
        str->appendf("%.2f", fRect.height());
    } else {
        str->appendf("X");
    }
    str->appendf(") ");
}
#endif

bool SkImageFilter::CropRect::applyTo(const SkIRect& imageBounds, const Context& ctx,
                                      SkIRect* cropped) const {
    *cropped = imageBounds;
    if (fFlags) {
        SkRect devCropR;
        ctx.ctm().mapRect(&devCropR, fRect);
        const SkIRect devICropR = devCropR.roundOut();

        // Compute the left/top first, in case we have to read them to compute right/bottom
        if (fFlags & kHasLeft_CropEdge) {
            cropped->fLeft = devICropR.fLeft;
        }
        if (fFlags & kHasTop_CropEdge) {
            cropped->fTop = devICropR.fTop;
        }
        if (fFlags & kHasWidth_CropEdge) {
            cropped->fRight = cropped->fLeft + devICropR.width();
        }
        if (fFlags & kHasHeight_CropEdge) {
            cropped->fBottom = cropped->fTop + devICropR.height();
        }
    }
    return cropped->intersect(ctx.clipBounds());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t next_image_filter_unique_id() {
    static int32_t gImageFilterUniqueID;

    // Never return 0.
    int32_t id;
    do {
        id = sk_atomic_inc(&gImageFilterUniqueID) + 1;
    } while (0 == id);
    return id;
}

struct SkImageFilter::Cache::Key {
    Key(const uint32_t uniqueID, const SkMatrix& matrix, const SkIRect& clipBounds, uint32_t srcGenID)
      : fUniqueID(uniqueID), fMatrix(matrix), fClipBounds(clipBounds), fSrcGenID(srcGenID) {
        // Assert that Key is tightly-packed, since it is hashed.
        SK_COMPILE_ASSERT(sizeof(Key) == sizeof(uint32_t) + sizeof(SkMatrix) + sizeof(SkIRect) +
                                         sizeof(uint32_t), image_filter_key_tight_packing);
        fMatrix.getType();  // force initialization of type, so hashes match
    }
    uint32_t fUniqueID;
    SkMatrix fMatrix;
    SkIRect fClipBounds;
    uint32_t fSrcGenID;
    bool operator==(const Key& other) const {
        return fUniqueID == other.fUniqueID
            && fMatrix == other.fMatrix
            && fClipBounds == other.fClipBounds
            && fSrcGenID == other.fSrcGenID;
    }
};

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
    const int count = buffer.readInt();
    if (!buffer.validate(count >= 0)) {
        return false;
    }
    if (!buffer.validate(expectedCount < 0 || count == expectedCount)) {
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
    if (buffer.isVersionLT(SkReadBuffer::kImageFilterNoUniqueID_Version)) {

        (void) buffer.readUInt();
    }
    return buffer.isValid();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImageFilter::SkImageFilter(int inputCount, SkImageFilter** inputs, const CropRect* cropRect)
  : fInputCount(inputCount),
    fInputs(new SkImageFilter*[inputCount]),
    fUsesSrcInput(false),
    fCropRect(cropRect ? *cropRect : CropRect(SkRect(), 0x0)),
    fUniqueID(next_image_filter_unique_id()) {
    for (int i = 0; i < inputCount; ++i) {
        if (NULL == inputs[i] || inputs[i]->usesSrcInput()) {
            fUsesSrcInput = true;
        }
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

SkImageFilter::SkImageFilter(int inputCount, SkReadBuffer& buffer)
  : fUsesSrcInput(false)
  , fUniqueID(next_image_filter_unique_id()) {
    Common common;
    if (common.unflatten(buffer, inputCount)) {
        fCropRect = common.cropRect();
        fInputCount = common.inputCount();
        fInputs = SkNEW_ARRAY(SkImageFilter*, fInputCount);
        common.detachInputs(fInputs);
        for (int i = 0; i < fInputCount; ++i) {
            if (NULL == fInputs[i] || fInputs[i]->usesSrcInput()) {
                fUsesSrcInput = true;
            }
        }
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
    SkASSERT(result);
    SkASSERT(offset);
    uint32_t srcGenID = fUsesSrcInput ? src.getGenerationID() : 0;
    Cache::Key key(fUniqueID, context.ctm(), context.clipBounds(), srcGenID);
    if (context.cache()) {
        if (context.cache()->get(key, result, offset)) {
            return true;
        }
    }
    /*
     *  Give the proxy first shot at the filter. If it returns false, ask
     *  the filter to do it.
     */
    if ((proxy && proxy->filterImage(this, src, context, result, offset)) ||
        this->onFilterImage(proxy, src, context, result, offset)) {
        if (context.cache()) {
            context.cache()->set(key, *result, *offset);
        }
        return true;
    }
    return false;
}

bool SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                 SkIRect* dst) const {
    SkASSERT(dst);
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
    return this->asFragmentProcessor(NULL, NULL, NULL, SkMatrix::I(), SkIRect());
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

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag,
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    SkAutoTUnref<GrTexture> dst(context->textureProvider()->createApproxTexture(desc));
    if (!dst) {
        return false;
    }

    // setup new clip
    GrClip clip(dstRect);

    GrFragmentProcessor* fp;
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    bounds.offset(-srcOffset);
    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    GrPaint paint;
    if (this->asFragmentProcessor(&fp, paint.getProcessorDataManager(), srcTexture, matrix, bounds)) {
        SkASSERT(fp);
        paint.addColorProcessor(fp)->unref();

        GrDrawContext* drawContext = context->drawContext();
        if (drawContext) {
            drawContext->drawNonAARectToRect(dst->asRenderTarget(), clip, paint, SkMatrix::I(),
                                             dstRect, srcRect);

            WrapTexture(dst, bounds.width(), bounds.height(), result);
            return true;
        }
    }
#endif
    return false;
}

bool SkImageFilter::applyCropRect(const Context& ctx, const SkBitmap& src,
                                  const SkIPoint& srcOffset, SkIRect* bounds) const {
    SkIRect srcBounds;
    src.getBounds(&srcBounds);
    srcBounds.offset(srcOffset);
    return fCropRect.applyTo(srcBounds, ctx, bounds);
}

bool SkImageFilter::applyCropRect(const Context& ctx, Proxy* proxy, const SkBitmap& src,
                                  SkIPoint* srcOffset, SkIRect* bounds, SkBitmap* dst) const {
    SkIRect srcBounds;
    src.getBounds(&srcBounds);
    srcBounds.offset(*srcOffset);
    if (!fCropRect.applyTo(srcBounds, ctx, bounds)) {
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
        *dst = src;
        return true;
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

bool SkImageFilter::asFragmentProcessor(GrFragmentProcessor**, GrProcessorDataManager*, GrTexture*,
                                        const SkMatrix&, const SkIRect&) const {
    return false;
}

SkImageFilter* SkImageFilter::CreateMatrixFilter(const SkMatrix& matrix,
                                                 SkFilterQuality filterQuality,
                                                 SkImageFilter* input) {
    return SkMatrixImageFilter::Create(matrix, filterQuality, input);
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

    if (this->canFilterImageGPU()) {
        return this->filterImageGPU(proxy, src, ctx, result, offset);
    } else {
        if (this->filterImage(proxy, src, ctx, result, offset)) {
            if (!result->getTexture()) {
                const SkImageInfo info = result->info();
                if (kUnknown_SkColorType == info.colorType()) {
                    return false;
                }
                SkAutoTUnref<GrTexture> resultTex(GrRefCachedBitmapTexture(context, *result, NULL));
                result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, resultTex)))->unref();
            }
            return true;
        } else {
            return false;
        }
    }
}
#endif

namespace {

class CacheImpl : public SkImageFilter::Cache {
public:
    CacheImpl(size_t maxBytes) : fMaxBytes(maxBytes), fCurrentBytes(0) {
    }
    virtual ~CacheImpl() {
        SkTDynamicHash<Value, Key>::Iter iter(&fLookup);

        while (!iter.done()) {
            Value* v = &*iter;
            ++iter;
            delete v;
        }
    }
    struct Value {
        Value(const Key& key, const SkBitmap& bitmap, const SkIPoint& offset)
            : fKey(key), fBitmap(bitmap), fOffset(offset) {}
        Key fKey;
        SkBitmap fBitmap;
        SkIPoint fOffset;
        static const Key& GetKey(const Value& v) {
            return v.fKey;
        }
        static uint32_t Hash(const Key& key) {
            return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
        }
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Value);
    };
    bool get(const Key& key, SkBitmap* result, SkIPoint* offset) const override {
        SkAutoMutexAcquire mutex(fMutex);
        if (Value* v = fLookup.find(key)) {
            *result = v->fBitmap;
            *offset = v->fOffset;
            if (v != fLRU.head()) {
                fLRU.remove(v);
                fLRU.addToHead(v);
            }
            return true;
        }
        return false;
    }
    void set(const Key& key, const SkBitmap& result, const SkIPoint& offset) override {
        SkAutoMutexAcquire mutex(fMutex);
        if (Value* v = fLookup.find(key)) {
            removeInternal(v);
        }
        Value* v = new Value(key, result, offset);
        fLookup.add(v);
        fLRU.addToHead(v);
        fCurrentBytes += result.getSize();
        while (fCurrentBytes > fMaxBytes) {
            Value* tail = fLRU.tail();
            SkASSERT(tail);
            if (tail == v) {
                break;
            }
            removeInternal(tail);
        }
    }

    void purge() override {
        SkAutoMutexAcquire mutex(fMutex);
        while (fCurrentBytes > 0) {
            Value* tail = fLRU.tail();
            SkASSERT(tail);
            this->removeInternal(tail);
        }
    }

private:
    void removeInternal(Value* v) {
        fCurrentBytes -= v->fBitmap.getSize();
        fLRU.remove(v);
        fLookup.remove(v->fKey);
        delete v;
    }
private:
    SkTDynamicHash<Value, Key>         fLookup;
    mutable SkTInternalLList<Value>    fLRU;
    size_t                             fMaxBytes;
    size_t                             fCurrentBytes;
    mutable SkMutex                    fMutex;
};

SkImageFilter::Cache* CreateCache() {
    return SkImageFilter::Cache::Create(kDefaultCacheSize);
}

} // namespace

SkImageFilter::Cache* SkImageFilter::Cache::Create(size_t maxBytes) {
    return SkNEW_ARGS(CacheImpl, (maxBytes));
}

SK_DECLARE_STATIC_LAZY_PTR(SkImageFilter::Cache, cache, CreateCache);

SkImageFilter::Cache* SkImageFilter::Cache::Get() {
    return cache.get();
}

void SkImageFilter::PurgeCache() {
    cache.get()->purge();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBaseDevice* SkImageFilter::Proxy::createDevice(int w, int h) {
    SkBaseDevice::CreateInfo cinfo(SkImageInfo::MakeN32Premul(w, h),
                                   SkBaseDevice::kNever_TileUsage,
                                   kUnknown_SkPixelGeometry,
                                   true /*forImageFilter*/);
    SkBaseDevice* dev = fDevice->onCreateDevice(cinfo, NULL);
    if (NULL == dev) {
        const SkSurfaceProps surfaceProps(fDevice->fSurfaceProps.flags(),
                                          kUnknown_SkPixelGeometry);
        dev = SkBitmapDevice::Create(cinfo.fInfo, surfaceProps);
    }
    return dev;
}

bool SkImageFilter::Proxy::filterImage(const SkImageFilter* filter, const SkBitmap& src,
                                       const SkImageFilter::Context& ctx,
                                       SkBitmap* result, SkIPoint* offset) {
    return fDevice->filterImage(filter, src, ctx, result, offset);
}

