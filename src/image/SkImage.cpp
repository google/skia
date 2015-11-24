/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkImageShader.h"
#include "SkImage_Base.h"
#include "SkNextID.h"
#include "SkPixelRef.h"
#include "SkPixelSerializer.h"
#include "SkReadPixelsRec.h"
#include "SkString.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrTexture.h"
#include "GrContext.h"
#include "SkImage_Gpu.h"
#endif

SkImage::SkImage(int width, int height, uint32_t uniqueID)
    : fWidth(width)
    , fHeight(height)
    , fUniqueID(kNeedNewImageUniqueID == uniqueID ? SkNextID::ImageID() : uniqueID)
{
    SkASSERT(width > 0);
    SkASSERT(height > 0);
}

const void* SkImage::peekPixels(SkImageInfo* info, size_t* rowBytes) const {
    SkImageInfo infoStorage;
    size_t rowBytesStorage;
    if (nullptr == info) {
        info = &infoStorage;
    }
    if (nullptr == rowBytes) {
        rowBytes = &rowBytesStorage;
    }
    return as_IB(this)->onPeekPixels(info, rowBytes);
}

bool SkImage::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           int srcX, int srcY, CachingHint chint) const {
    SkReadPixelsRec rec(dstInfo, dstPixels, dstRowBytes, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }
    return as_IB(this)->onReadPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, chint);
}

bool SkImage::scalePixels(const SkPixmap& dst, SkFilterQuality quality, CachingHint chint) const {
    if (this->width() == dst.width() && this->height() == dst.height()) {
        return this->readPixels(dst, 0, 0, chint);
    }

    // Idea: If/when SkImageGenerator supports a native-scaling API (where the generator itself
    //       can scale more efficiently) we should take advantage of it here.
    //
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm, chint)) {
        bm.lockPixels();
        SkPixmap pmap;
        // Note: By calling the pixmap scaler, we never cache the final result, so the chint
        //       is (currently) only being applied to the getROPixels. If we get a request to
        //       also attempt to cache the final (scaled) result, we would add that logic here.
        //
        return bm.peekPixels(&pmap) && pmap.scalePixels(dst, quality);
    }
    return false;
}

void SkImage::preroll(GrContext* ctx) const {
    // For now, and to maintain parity w/ previous pixelref behavior, we just force the image
    // to produce a cached raster-bitmap form, so that drawing to a raster canvas should be fast.
    //
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm)) {
        bm.lockPixels();
        bm.unlockPixels();
    }
}

SkImage* SkImage::applyFilter(SkImageFilter* filter, SkIPoint* offset,
                              bool forceResultToOriginalSize) const {
    if (!filter) {
        return nullptr;
    }

    SkIPoint offsetStorage;
    if (!offset) {
        offset = &offsetStorage;
    }
    return as_IB(this)->onApplyFilter(filter, offset, forceResultToOriginalSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkImageFilter.h"
#include "SkBitmapDevice.h"

static SkIRect compute_fast_ibounds(SkImageFilter* filter, const SkIRect& srcBounds) {
    SkRect fastBounds;
    fastBounds.set(srcBounds);
    filter->computeFastBounds(fastBounds, &fastBounds);
    return fastBounds.roundOut();
}

class SkRasterImageFilterProxy : public SkImageFilter::Proxy {
public:
    SkBaseDevice* createDevice(int width, int height) override {
        return SkBitmapDevice::Create(SkImageInfo::MakeN32Premul(width, height));
    }

    bool filterImage(const SkImageFilter*, const SkBitmap&, const SkImageFilter::Context&,
                     SkBitmap*, SkIPoint*) override {
        return false;
    }
};

SkImage* SkImage_Base::onApplyFilter(SkImageFilter* filter, SkIPoint* offsetResult,
                                     bool forceResultToOriginalSize) const {
    SkBitmap src;
    if (!this->getROPixels(&src)) {
        return nullptr;
    }

    const SkIRect srcBounds = SkIRect::MakeWH(this->width(), this->height());

    if (forceResultToOriginalSize) {
        const SkIRect clipBounds = srcBounds;
        SkRasterImageFilterProxy proxy;
        SkImageFilter::Context ctx(SkMatrix::I(), clipBounds, SkImageFilter::Cache::Get(),
                                   SkImageFilter::kExact_SizeConstraint);

        SkBitmap dst;
        if (filter->filterImage(&proxy, src, ctx, &dst, offsetResult)) {
            dst.setImmutable();
            return SkImage::NewFromBitmap(dst);
        }
    } else {
        const SkIRect dstR = compute_fast_ibounds(filter, srcBounds);

        SkImageInfo info = SkImageInfo::MakeN32Premul(dstR.width(), dstR.height());
        SkAutoTUnref<SkSurface> surface(this->onNewSurface(info));

        SkPaint paint;
        paint.setImageFilter(filter);
        surface->getCanvas()->drawImage(this, SkIntToScalar(-dstR.x()), SkIntToScalar(-dstR.y()),
                                        &paint);

        offsetResult->set(dstR.x(), dstR.y());
        return surface->newImageSnapshot();
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkShader* SkImage::newShader(SkShader::TileMode tileX,
                             SkShader::TileMode tileY,
                             const SkMatrix* localMatrix) const {
    return SkImageShader::Create(this, tileX, tileY, localMatrix);
}

SkData* SkImage::encode(SkImageEncoder::Type type, int quality) const {
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm)) {
        return SkImageEncoder::EncodeData(bm, type, quality);
    }
    return nullptr;
}

namespace {

class DefaultSerializer :  public SkPixelSerializer {
protected:
    bool onUseEncodedData(const void *data, size_t len) override {
        return true;
    }

    SkData* onEncodePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes) override {
        return SkImageEncoder::EncodeData(info, pixels, rowBytes, SkImageEncoder::kPNG_Type, 100);
    }
};

} // anonymous namespace

SkData* SkImage::encode(SkPixelSerializer* serializer) const {
    DefaultSerializer defaultSerializer;
    SkPixelSerializer* effectiveSerializer = serializer ? serializer : &defaultSerializer;

    SkAutoTUnref<SkData> encoded(this->refEncoded());
    if (encoded && effectiveSerializer->useEncodedData(encoded->data(), encoded->size())) {
        return encoded.detach();
    }

    SkBitmap bm;
    SkAutoPixmapUnlock apu;
    if (as_IB(this)->getROPixels(&bm) && bm.requestLock(&apu)) {
        const SkPixmap& pmap = apu.pixmap();
        return effectiveSerializer->encodePixels(pmap.info(), pmap.addr(), pmap.rowBytes());
    }

    return nullptr;
}

SkData* SkImage::refEncoded() const {
    return as_IB(this)->onRefEncoded();
}

SkImage* SkImage::NewFromEncoded(SkData* encoded, const SkIRect* subset) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    SkImageGenerator* generator = SkImageGenerator::NewFromEncoded(encoded);
    return generator ? SkImage::NewFromGenerator(generator, subset) : nullptr;
}

const char* SkImage::toString(SkString* str) const {
    str->appendf("image: (id:%d (%d, %d) %s)", this->uniqueID(), this->width(), this->height(),
                 this->isOpaque() ? "opaque" : "");
    return str->c_str();
}

SkImage* SkImage::newSubset(const SkIRect& subset) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    // optimization : return self if the subset == our bounds
    if (bounds == subset) {
        return SkRef(const_cast<SkImage*>(this));
    }
    return as_IB(this)->onNewSubset(subset);
}

#if SK_SUPPORT_GPU

GrTexture* SkImage::getTexture() const {
    return as_IB(this)->peekTexture();
}

bool SkImage::isTextureBacked() const { return SkToBool(as_IB(this)->getTexture()); }

GrBackendObject SkImage::getTextureHandle(bool flushPendingGrContextIO) const {
    GrTexture* texture = as_IB(this)->getTexture();
    if (texture) {
        GrContext* context = texture->getContext();
        if (context) {            
            if (flushPendingGrContextIO) {
                context->prepareSurfaceForExternalIO(texture);
            }
        }
        return texture->getTextureHandle();
    }
    return 0;
}

#else

GrTexture* SkImage::getTexture() const { return nullptr; }

bool SkImage::isTextureBacked() const { return false; }

GrBackendObject SkImage::getTextureHandle(bool) const { return 0; }

#endif

///////////////////////////////////////////////////////////////////////////////

static bool raster_canvas_supports(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:
            return kUnpremul_SkAlphaType != info.alphaType();
        case kRGB_565_SkColorType:
            return true;
        case kAlpha_8_SkColorType:
            return true;
        default:
            break;
    }
    return false;
}

SkImage_Base::SkImage_Base(int width, int height, uint32_t uniqueID)
    : INHERITED(width, height, uniqueID)
    , fAddedToCache(false)
{}

SkImage_Base::~SkImage_Base() {
    if (fAddedToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

bool SkImage_Base::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                int srcX, int srcY, CachingHint) const {
    if (!raster_canvas_supports(dstInfo)) {
        return false;
    }

    SkBitmap bm;
    bm.installPixels(dstInfo, dstPixels, dstRowBytes);
    SkCanvas canvas(bm);

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawImage(this, -SkIntToScalar(srcX), -SkIntToScalar(srcY), &paint);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage::peekPixels(SkPixmap* pmap) const {
    SkImageInfo info;
    size_t rowBytes;
    const void* pixels = this->peekPixels(&info, &rowBytes);
    if (pixels) {
        if (pmap) {
            pmap->reset(info, pixels, rowBytes);
        }
        return true;
    }
    return false;
}

bool SkImage::readPixels(const SkPixmap& pmap, int srcX, int srcY, CachingHint chint) const {
    return this->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), srcX, srcY, chint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewFromBitmap(const SkBitmap& bm) {
    SkPixelRef* pr = bm.pixelRef();
    if (nullptr == pr) {
        return nullptr;
    }

#if SK_SUPPORT_GPU
    if (GrTexture* tex = pr->getTexture()) {
        SkAutoTUnref<GrTexture> unrefCopy;
        if (!bm.isImmutable()) {
            const bool notBudgeted = false;
            tex = GrDeepCopyTexture(tex, notBudgeted);
            if (nullptr == tex) {
                return nullptr;
            }
            unrefCopy.reset(tex);
        }
        const SkImageInfo info = bm.info();
        return new SkImage_Gpu(info.width(), info.height(), bm.getGenerationID(), info.alphaType(),
                               tex, SkSurface::kNo_Budgeted);
    }
#endif

    // This will check for immutable (share or copy)
    return SkNewImageFromRasterBitmap(bm);
}

bool SkImage::asLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    return as_IB(this)->onAsLegacyBitmap(bitmap, mode);
}

bool SkImage_Base::onAsLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = SkImageInfo::MakeN32(this->width(), this->height(),
                                    this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }
    if (!this->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0)) {
        bitmap->reset();
        return false;
    }

    if (kRO_LegacyBitmapMode == mode) {
        bitmap->setImmutable();
    }
    return true;
}

SkImage* SkImage::NewFromPicture(const SkPicture* picture, const SkISize& dimensions,
                                 const SkMatrix* matrix, const SkPaint* paint) {
    if (!picture) {
        return nullptr;
    }
    return NewFromGenerator(SkImageGenerator::NewFromPicture(dimensions, picture, matrix, paint));
}

bool SkImage::isLazyGenerated() const {
    return as_IB(this)->onIsLazyGenerated();
}

//////////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU

SkImage* SkImage::NewFromTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType,
                                 TextureReleaseProc, ReleaseContext) {
    return nullptr;
}

SkImage* SkImage::NewFromAdoptedTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType) {
    return nullptr;
}

SkImage* SkImage::NewFromTextureCopy(GrContext*, const GrBackendTextureDesc&, SkAlphaType) {
    return nullptr;
}

#endif
