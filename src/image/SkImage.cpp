/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkColorSpace_Base.h"
#include "SkCrossContextImageData.h"
#include "SkData.h"
#include "SkImageEncoder.h"
#include "SkImageFilter.h"
#include "SkImageFilterCache.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
#include "SkImageShader.h"
#include "SkImage_Base.h"
#include "SkNextID.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkPixelSerializer.h"
#include "SkRGBAToYUV.h"
#include "SkReadPixelsRec.h"
#include "SkSpecialImage.h"
#include "SkStream.h"
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

bool SkImage::peekPixels(SkPixmap* pm) const {
    SkPixmap tmp;
    if (!pm) {
        pm = &tmp;
    }
    return as_IB(this)->onPeekPixels(pm);
}

bool SkImage::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           int srcX, int srcY, CachingHint chint) const {
    return as_IB(this)->onReadPixels(dstInfo, dstPixels, dstRowBytes, srcX, srcY, chint);
}

bool SkImage::scalePixels(const SkPixmap& dst, SkFilterQuality quality, CachingHint chint) const {
    if (this->width() == dst.width() && this->height() == dst.height()) {
        return this->readPixels(dst, 0, 0, chint);
    }

    // Idea: If/when SkImageGenerator supports a native-scaling API (where the generator itself
    //       can scale more efficiently) we should take advantage of it here.
    //
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm, dst.info().colorSpace(), chint)) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////

SkAlphaType SkImage::alphaType() const {
    return as_IB(this)->onAlphaType();
}

SkColorSpace* SkImage::colorSpace() const {
    return as_IB(this)->onImageInfo().colorSpace();
}

sk_sp<SkColorSpace> SkImage::refColorSpace() const {
    return as_IB(this)->onImageInfo().refColorSpace();
}

sk_sp<SkShader> SkImage::makeShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                                    const SkMatrix* localMatrix) const {
    return SkImageShader::Make(sk_ref_sp(const_cast<SkImage*>(this)), tileX, tileY, localMatrix);
}

SkData* SkImage::encode(SkEncodedImageFormat type, int quality) const {
    SkBitmap bm;
    SkColorSpace* legacyColorSpace = nullptr;
    if (as_IB(this)->getROPixels(&bm, legacyColorSpace)) {
        SkDynamicMemoryWStream buf;
        return SkEncodeImage(&buf, bm, type, quality) ? buf.detachAsData().release() : nullptr;
    }
    return nullptr;
}

SkData* SkImage::encode(SkPixelSerializer* serializer) const {
    sk_sp<SkData> encoded(this->refEncoded());
    if (encoded &&
        (!serializer || serializer->useEncodedData(encoded->data(), encoded->size()))) {
        return encoded.release();
    }

    SkBitmap bm;
    SkAutoPixmapUnlock apu;
    SkColorSpace* legacyColorSpace = nullptr;
    if (as_IB(this)->getROPixels(&bm, legacyColorSpace) &&
        bm.requestLock(&apu)) {
        if (serializer) {
            return serializer->encode(apu.pixmap());
        } else {
            SkDynamicMemoryWStream buf;
            return SkEncodeImage(&buf, apu.pixmap(), SkEncodedImageFormat::kPNG, 100)
                   ? buf.detachAsData().release() : nullptr;
        }
    }

    return nullptr;
}

SkData* SkImage::refEncoded() const {
    GrContext* ctx = nullptr;   // should we allow the caller to pass in a ctx?
    return as_IB(this)->onRefEncoded(ctx);
}

sk_sp<SkImage> SkImage::MakeFromEncoded(sk_sp<SkData> encoded, const SkIRect* subset) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    return SkImage::MakeFromGenerator(SkImageGenerator::MakeFromEncoded(encoded), subset);
}

const char* SkImage::toString(SkString* str) const {
    str->appendf("image: (id:%d (%d, %d) %s)", this->uniqueID(), this->width(), this->height(),
                 this->isOpaque() ? "opaque" : "");
    return str->c_str();
}

sk_sp<SkImage> SkImage::makeSubset(const SkIRect& subset) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    // optimization : return self if the subset == our bounds
    if (bounds == subset) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }
    return as_IB(this)->onMakeSubset(subset);
}

#if SK_SUPPORT_GPU

GrTexture* SkImage::getTexture() const {
    return as_IB(this)->onGetTexture();
}

bool SkImage::isTextureBacked() const { return SkToBool(as_IB(this)->peekProxy()); }

GrBackendObject SkImage::getTextureHandle(bool flushPendingGrContextIO,
                                          GrSurfaceOrigin* origin) const {
    return as_IB(this)->onGetTextureHandle(flushPendingGrContextIO, origin);
}

#else

GrTexture* SkImage::getTexture() const { return nullptr; }

bool SkImage::isTextureBacked() const { return false; }

GrBackendObject SkImage::getTextureHandle(bool, GrSurfaceOrigin*) const { return 0; }

#endif

///////////////////////////////////////////////////////////////////////////////

SkImage_Base::SkImage_Base(int width, int height, uint32_t uniqueID)
    : INHERITED(width, height, uniqueID)
    , fAddedToCache(false)
{}

SkImage_Base::~SkImage_Base() {
    if (fAddedToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

bool SkImage_Base::onReadYUV8Planes(const SkISize sizes[3], void* const planes[3],
                                    const size_t rowBytes[3], SkYUVColorSpace colorSpace) const {
    return SkRGBAToYUV(this, sizes, planes, rowBytes, colorSpace);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage::readPixels(const SkPixmap& pmap, int srcX, int srcY, CachingHint chint) const {
    return this->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), srcX, srcY, chint);
}

bool SkImage::readYUV8Planes(const SkISize sizes[3], void* const planes[3],
                             const size_t rowBytes[3], SkYUVColorSpace colorSpace) const {
    return as_IB(this)->onReadYUV8Planes(sizes, planes, rowBytes, colorSpace);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeFromBitmap(const SkBitmap& bm) {
    SkPixelRef* pr = bm.pixelRef();
    if (nullptr == pr) {
        return nullptr;
    }

    return SkMakeImageFromRasterBitmap(bm, kIfMutable_SkCopyPixelsMode);
}

bool SkImage::asLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    return as_IB(this)->onAsLegacyBitmap(bitmap, mode);
}

bool SkImage_Base::onAsLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode mode) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = this->onImageInfo().makeColorType(kN32_SkColorType).makeColorSpace(nullptr);
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

sk_sp<SkImage> SkImage::MakeFromPicture(sk_sp<SkPicture> picture, const SkISize& dimensions,
                                        const SkMatrix* matrix, const SkPaint* paint,
                                        BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    return MakeFromGenerator(SkImageGenerator::MakeFromPicture(dimensions, std::move(picture),
                                                               matrix, paint, bitDepth,
                                                               std::move(colorSpace)));
}

sk_sp<SkImage> SkImage::makeWithFilter(const SkImageFilter* filter, const SkIRect& subset,
                                       const SkIRect& clipBounds, SkIRect* outSubset,
                                       SkIPoint* offset) const {
    if (!filter || !outSubset || !offset || !this->bounds().contains(subset)) {
        return nullptr;
    }
    SkColorSpace* colorSpace = as_IB(this)->onImageInfo().colorSpace();
    sk_sp<SkSpecialImage> srcSpecialImage = SkSpecialImage::MakeFromImage(
        subset, sk_ref_sp(const_cast<SkImage*>(this)), colorSpace);
    if (!srcSpecialImage) {
        return nullptr;
    }

    sk_sp<SkImageFilterCache> cache(
        SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize));
    SkImageFilter::OutputProperties outputProperties(colorSpace);
    SkImageFilter::Context context(SkMatrix::I(), clipBounds, cache.get(), outputProperties);

    sk_sp<SkSpecialImage> result = filter->filterImage(srcSpecialImage.get(), context, offset);
    if (!result) {
        return nullptr;
    }

    *outSubset = SkIRect::MakeWH(result->width(), result->height());
    if (!outSubset->intersect(clipBounds.makeOffset(-offset->x(), -offset->y()))) {
        return nullptr;
    }
    offset->fX += outSubset->x();
    offset->fY += outSubset->y();

    // Note that here we're returning the special image's entire backing store, loose padding
    // and all!
    return result->asImage();
}

bool SkImage::isLazyGenerated() const {
    return as_IB(this)->onIsLazyGenerated();
}

bool SkImage::isAlphaOnly() const {
    return as_IB(this)->onImageInfo().colorType() == kAlpha_8_SkColorType;
}

sk_sp<SkImage> SkImage::makeColorSpace(sk_sp<SkColorSpace> target,
                                       SkTransferFunctionBehavior premulBehavior) const {
    if (SkTransferFunctionBehavior::kRespect == premulBehavior) {
        // TODO (msarett, brianosman): Implement this.
        return nullptr;
    }

    SkColorSpaceTransferFn fn;
    if (!target || !target->isNumericalTransferFn(&fn)) {
        return nullptr;
    }

    // No need to create a new image if:
    // (1) The color spaces are equal (nullptr is considered to be sRGB).
    // (2) The color type is kAlpha8.
    if ((!this->colorSpace() && target->isSRGB()) ||
            SkColorSpace::Equals(this->colorSpace(), target.get()) ||
            kAlpha_8_SkColorType == as_IB(this)->onImageInfo().colorType()) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }

    // TODO: We might consider making this a deferred conversion?
    return as_IB(this)->onMakeColorSpace(std::move(target));
}

//////////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU

sk_sp<SkImage> MakeTextureFromMipMap(GrContext*, const SkImageInfo&, const GrMipLevel* texels,
                                     int mipLevelCount, SkBudgeted, SkDestinationSurfaceColorMode) {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType,
                                        sk_sp<SkColorSpace>, TextureReleaseProc, ReleaseContext) {
    return nullptr;
}

size_t SkImage::getDeferredTextureImageData(const GrContextThreadSafeProxy&,
                                            const DeferredTextureImageUsageParams[],
                                            int paramCnt, void* buffer,
                                            SkColorSpace* dstColorSpace) const {
    return 0;
}

sk_sp<SkImage> SkImage::MakeFromDeferredTextureImageData(GrContext* context, const void*,
                                                         SkBudgeted) {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext*, const GrBackendTextureDesc&,
                                               SkAlphaType, sk_sp<SkColorSpace>) {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace space,
                                                const GrBackendObject yuvTextureHandles[3],
                                                const SkISize yuvSizes[3],
                                                GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> imageColorSpace) {
    return nullptr;
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext*, SkColorSpace* dstColorSpace) const {
    return nullptr;
}

std::unique_ptr<SkCrossContextImageData> SkCrossContextImageData::MakeFromEncoded(
        GrContext*, sk_sp<SkData> encoded, SkColorSpace* dstColorSpace) {
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(encoded));
    if (!image) {
        return nullptr;
    }
    // TODO: Force decode to raster here?
    return std::unique_ptr<SkCrossContextImageData>(new SkCrossContextImageData(std::move(image)));
}

sk_sp<SkImage> SkImage::MakeFromCrossContextImageData(
        GrContext*, std::unique_ptr<SkCrossContextImageData> ccid) {
    return ccid->fImage;
}

sk_sp<SkImage> SkImage::makeNonTextureImage() const {
    return sk_ref_sp(const_cast<SkImage*>(this));
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> MakeTextureFromMipMap(GrContext*, const SkImageInfo&, const GrMipLevel* texels,
                                     int mipLevelCount, SkBudgeted) {
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkImageDeserializer.h"

sk_sp<SkImage> SkImageDeserializer::makeFromData(SkData* data, const SkIRect* subset) {
    return SkImage::MakeFromEncoded(sk_ref_sp(data), subset);
}
sk_sp<SkImage> SkImageDeserializer::makeFromMemory(const void* data, size_t length,
                                                   const SkIRect* subset) {
    return SkImage::MakeFromEncoded(SkData::MakeWithCopy(data, length), subset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_pinAsTexture(const SkImage* image, GrContext* ctx) {
    SkASSERT(image);
    SkASSERT(ctx);
    return as_IB(image)->onPinAsTexture(ctx);
}

void SkImage_unpinAsTexture(const SkImage* image, GrContext* ctx) {
    SkASSERT(image);
    SkASSERT(ctx);
    as_IB(image)->onUnpinAsTexture(ctx);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImageMakeRasterCopyAndAssignColorSpace(const SkImage* src,
                                                        SkColorSpace* colorSpace) {
    // Read the pixels out of the source image, with no conversion
    SkImageInfo info = as_IB(src)->onImageInfo();
    if (kUnknown_SkColorType == info.colorType()) {
        SkDEBUGFAIL("Unexpected color type");
        return nullptr;
    }

    size_t rowBytes = info.minRowBytes();
    size_t size = info.getSafeSize(rowBytes);
    auto data = SkData::MakeUninitialized(size);
    if (!data) {
        return nullptr;
    }

    SkPixmap pm(info, data->writable_data(), rowBytes);
    if (!src->readPixels(pm, 0, 0, SkImage::kDisallow_CachingHint)) {
        return nullptr;
    }

    // Wrap them in a new image with a different color space
    return SkImage::MakeRasterData(info.makeColorSpace(sk_ref_sp(colorSpace)), data, rowBytes);
}
