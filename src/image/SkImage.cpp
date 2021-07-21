/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPicture.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkMipmapBuilder.h"
#include "src/core/SkNextID.h"
#include "src/core/SkSpecialImage.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkReadPixelsRec.h"
#include "src/image/SkRescaleAndReadPixels.h"
#include "src/shaders/SkImageShader.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/image/SkImage_Gpu.h"
#endif
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextThreadSafeProxy.h"

SkImage::SkImage(const SkImageInfo& info, uint32_t uniqueID)
        : fInfo(info)
        , fUniqueID(kNeedNewImageUniqueID == uniqueID ? SkNextID::ImageID() : uniqueID) {
    SkASSERT(info.width() > 0);
    SkASSERT(info.height() > 0);
}

bool SkImage::peekPixels(SkPixmap* pm) const {
    SkPixmap tmp;
    if (!pm) {
        pm = &tmp;
    }
    return as_IB(this)->onPeekPixels(pm);
}

bool SkImage::readPixels(GrDirectContext* dContext, const SkImageInfo& dstInfo, void* dstPixels,
                         size_t dstRowBytes, int srcX, int srcY, CachingHint chint) const {
    return as_IB(this)->onReadPixels(dContext, dstInfo, dstPixels, dstRowBytes, srcX, srcY, chint);
}

#ifndef SK_IMAGE_READ_PIXELS_DISABLE_LEGACY_API
bool SkImage::readPixels(const SkImageInfo& dstInfo, void* dstPixels,
                         size_t dstRowBytes, int srcX, int srcY, CachingHint chint) const {
    auto dContext = as_IB(this)->directContext();
    return this->readPixels(dContext, dstInfo, dstPixels, dstRowBytes, srcX, srcY, chint);
}
#endif

void SkImage::asyncRescaleAndReadPixels(const SkImageInfo& info,
                                        const SkIRect& srcRect,
                                        RescaleGamma rescaleGamma,
                                        RescaleMode rescaleMode,
                                        ReadPixelsCallback callback,
                                        ReadPixelsContext context) {
    if (!SkIRect::MakeWH(this->width(), this->height()).contains(srcRect) ||
        !SkImageInfoIsValid(info)) {
        callback(context, nullptr);
        return;
    }
    as_IB(this)->onAsyncRescaleAndReadPixels(
            info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

void SkImage::asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                              sk_sp<SkColorSpace> dstColorSpace,
                                              const SkIRect& srcRect,
                                              const SkISize& dstSize,
                                              RescaleGamma rescaleGamma,
                                              RescaleMode rescaleMode,
                                              ReadPixelsCallback callback,
                                              ReadPixelsContext context) {
    if (!SkIRect::MakeWH(this->width(), this->height()).contains(srcRect) || dstSize.isZero() ||
        (dstSize.width() & 0b1) || (dstSize.height() & 0b1)) {
        callback(context, nullptr);
        return;
    }
    as_IB(this)->onAsyncRescaleAndReadPixelsYUV420(yuvColorSpace,
                                                   std::move(dstColorSpace),
                                                   srcRect,
                                                   dstSize,
                                                   rescaleGamma,
                                                   rescaleMode,
                                                   callback,
                                                   context);
}

bool SkImage::scalePixels(const SkPixmap& dst, const SkSamplingOptions& sampling,
                          CachingHint chint) const {
    // Context TODO: Elevate GrDirectContext requirement to public API.
    auto dContext = as_IB(this)->directContext();
    if (this->width() == dst.width() && this->height() == dst.height()) {
        return this->readPixels(dContext, dst, 0, 0, chint);
    }

    // Idea: If/when SkImageGenerator supports a native-scaling API (where the generator itself
    //       can scale more efficiently) we should take advantage of it here.
    //
    SkBitmap bm;
    if (as_IB(this)->getROPixels(dContext, &bm, chint)) {
        SkPixmap pmap;
        // Note: By calling the pixmap scaler, we never cache the final result, so the chint
        //       is (currently) only being applied to the getROPixels. If we get a request to
        //       also attempt to cache the final (scaled) result, we would add that logic here.
        //
        return bm.peekPixels(&pmap) && pmap.scalePixels(dst, sampling);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColorType SkImage::colorType() const { return fInfo.colorType(); }

SkAlphaType SkImage::alphaType() const { return fInfo.alphaType(); }

SkColorSpace* SkImage::colorSpace() const { return fInfo.colorSpace(); }

sk_sp<SkColorSpace> SkImage::refColorSpace() const { return fInfo.refColorSpace(); }

sk_sp<SkShader> SkImage::makeShader(SkTileMode tmx, SkTileMode tmy,
                                    const SkSamplingOptions& sampling,
                                    const SkMatrix* localMatrix) const {
    return SkImageShader::Make(sk_ref_sp(const_cast<SkImage*>(this)), tmx, tmy,
                               sampling, localMatrix);
}

sk_sp<SkData> SkImage::encodeToData(SkEncodedImageFormat type, int quality) const {
    // Context TODO: Elevate GrDirectContext requirement to public API.
    auto dContext = as_IB(this)->directContext();
    SkBitmap bm;
    if (as_IB(this)->getROPixels(dContext, &bm)) {
        return SkEncodeBitmap(bm, type, quality);
    }
    return nullptr;
}

sk_sp<SkData> SkImage::encodeToData() const {
    if (auto encoded = this->refEncodedData()) {
        return encoded;
    }

    return this->encodeToData(SkEncodedImageFormat::kPNG, 100);
}

sk_sp<SkData> SkImage::refEncodedData() const {
    return sk_sp<SkData>(as_IB(this)->onRefEncoded());
}

sk_sp<SkImage> SkImage::MakeFromEncoded(sk_sp<SkData> encoded) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    return SkImage::MakeFromGenerator(SkImageGenerator::MakeFromEncoded(std::move(encoded)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::makeSubset(const SkIRect& subset, GrDirectContext* direct) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

#if SK_SUPPORT_GPU
    auto myContext = as_IB(this)->context();
    // This check is also performed in the subclass, but we do it here for the short-circuit below.
    if (myContext && !myContext->priv().matches(direct)) {
        return nullptr;
    }
#endif

    // optimization : return self if the subset == our bounds
    if (bounds == subset) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }

    return as_IB(this)->onMakeSubset(subset, direct);
}

#if SK_SUPPORT_GPU

bool SkImage::isTextureBacked() const { return as_IB(this)->onIsTextureBacked(); }

size_t SkImage::textureSize() const { return as_IB(this)->onTextureSize(); }

GrBackendTexture SkImage::getBackendTexture(bool flushPendingGrContextIO,
                                            GrSurfaceOrigin* origin) const {
    return as_IB(this)->onGetBackendTexture(flushPendingGrContextIO, origin);
}

bool SkImage::isValid(GrRecordingContext* rContext) const {
    if (rContext && rContext->abandoned()) {
        return false;
    }
    return as_IB(this)->onIsValid(rContext);
}

GrSemaphoresSubmitted SkImage::flush(GrDirectContext* dContext, const GrFlushInfo& flushInfo) {
    return as_IB(this)->onFlush(dContext, flushInfo);
}

void SkImage::flushAndSubmit(GrDirectContext* dContext) {
    this->flush(dContext, {});
    dContext->submit();
}

#else

bool SkImage::isTextureBacked() const { return false; }

GrBackendTexture SkImage::getBackendTexture(bool flushPendingGrContextIO,
                                            GrSurfaceOrigin* origin) const {
    return GrBackendTexture(); // invalid
}

bool SkImage::isValid(GrRecordingContext* rContext) const {
    if (rContext) {
        return false;
    }
    return as_IB(this)->onIsValid(nullptr);
}

GrSemaphoresSubmitted SkImage::flush(GrDirectContext*, const GrFlushInfo&) {
    return GrSemaphoresSubmitted::kNo;
}

void SkImage::flushAndSubmit(GrDirectContext*) {}

#endif

///////////////////////////////////////////////////////////////////////////////

SkImage_Base::SkImage_Base(const SkImageInfo& info, uint32_t uniqueID)
        : INHERITED(info, uniqueID), fAddedToRasterCache(false) {}

SkImage_Base::~SkImage_Base() {
    if (fAddedToRasterCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

void SkImage_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                               const SkIRect& origSrcRect,
                                               RescaleGamma rescaleGamma,
                                               RescaleMode rescaleMode,
                                               ReadPixelsCallback callback,
                                               ReadPixelsContext context) {
    SkBitmap src;
    SkPixmap peek;
    SkIRect srcRect;
    if (this->peekPixels(&peek)) {
        src.installPixels(peek);
        srcRect = origSrcRect;
    } else {
        // Context TODO: Elevate GrDirectContext requirement to public API.
        auto dContext = as_IB(this)->directContext();
        src.setInfo(this->imageInfo().makeDimensions(origSrcRect.size()));
        src.allocPixels();
        if (!this->readPixels(dContext, src.pixmap(), origSrcRect.x(), origSrcRect.y())) {
            callback(context, nullptr);
            return;
        }
        srcRect = SkIRect::MakeSize(src.dimensions());
    }
    return SkRescaleAndReadPixels(src, info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

void SkImage_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                     sk_sp<SkColorSpace> dstColorSpace,
                                                     const SkIRect& srcRect,
                                                     const SkISize& dstSize,
                                                     RescaleGamma,
                                                     RescaleMode,
                                                     ReadPixelsCallback callback,
                                                     ReadPixelsContext context) {
    // TODO: Call non-YUV asyncRescaleAndReadPixels and then make our callback convert to YUV and
    // call client's callback.
    callback(context, nullptr);
}

#if SK_SUPPORT_GPU
std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Base::asView(GrRecordingContext* context,
                                                                 GrMipmapped mipmapped,
                                                                 GrImageTexGenPolicy policy) const {
    if (!context) {
        return {};
    }
    if (!context->priv().caps()->mipmapSupport() || this->dimensions().area() <= 1) {
        mipmapped = GrMipmapped::kNo;
    }
    return this->onAsView(context, mipmapped, policy);
}

std::unique_ptr<GrFragmentProcessor> SkImage_Base::asFragmentProcessor(
        GrRecordingContext* rContext,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    if (!rContext) {
        return {};
    }
    if (sampling.useCubic && !GrValidCubicResampler(sampling.cubic)) {
        return {};
    }
    if (sampling.mipmap != SkMipmapMode::kNone &&
        (!rContext->priv().caps()->mipmapSupport() || this->dimensions().area() <= 1)) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    return this->onAsFragmentProcessor(rContext, sampling, tileModes, m, subset, domain);
}

std::unique_ptr<GrFragmentProcessor> SkImage_Base::MakeFragmentProcessorFromView(
        GrRecordingContext* rContext,
        GrSurfaceProxyView view,
        SkAlphaType at,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) {
    if (!view) {
        return nullptr;
    }
    const GrCaps& caps = *rContext->priv().caps();
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    if (sampling.useCubic) {
        if (subset) {
            if (domain) {
                return GrBicubicEffect::MakeSubset(std::move(view),
                                                   at,
                                                   m,
                                                   wmx,
                                                   wmy,
                                                   *subset,
                                                   *domain,
                                                   sampling.cubic,
                                                   GrBicubicEffect::Direction::kXY,
                                                   *rContext->priv().caps());
            }
            return GrBicubicEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               wmx,
                                               wmy,
                                               *subset,
                                               sampling.cubic,
                                               GrBicubicEffect::Direction::kXY,
                                               *rContext->priv().caps());
        }
        return GrBicubicEffect::Make(std::move(view),
                                     at,
                                     m,
                                     wmx,
                                     wmy,
                                     sampling.cubic,
                                     GrBicubicEffect::Direction::kXY,
                                     *rContext->priv().caps());
    }
    if (view.proxy()->asTextureProxy()->mipmapped() == GrMipmapped::kNo) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    GrSamplerState sampler(wmx, wmy, sampling.filter, sampling.mipmap);
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               sampler,
                                               *subset,
                                               *domain,
                                               caps);
        }
        return GrTextureEffect::MakeSubset(std::move(view),
                                           at,
                                           m,
                                           sampler,
                                           *subset,
                                           caps);
    } else {
        return GrTextureEffect::Make(std::move(view), at, m, sampler, caps);
    }
}

GrSurfaceProxyView SkImage_Base::FindOrMakeCachedMipmappedView(GrRecordingContext* rContext,
                                                               GrSurfaceProxyView view,
                                                               uint32_t imageUniqueID) {
    SkASSERT(rContext);
    SkASSERT(imageUniqueID != SK_InvalidUniqueID);

    if (!view || view.proxy()->asTextureProxy()->mipmapped() == GrMipmapped::kYes) {
        return view;
    }
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, imageUniqueID, SkIRect::MakeSize(view.dimensions()));
    SkASSERT(baseKey.isValid());
    GrUniqueKey mipmappedKey;
    static const GrUniqueKey::Domain kMipmappedDomain = GrUniqueKey::GenerateDomain();
    {  // No extra values beyond the domain are required. Must name the var to please
       // clang-tidy.
        GrUniqueKey::Builder b(&mipmappedKey, baseKey, kMipmappedDomain, 0);
    }
    SkASSERT(mipmappedKey.isValid());
    if (sk_sp<GrTextureProxy> cachedMippedView =
                proxyProvider->findOrCreateProxyByUniqueKey(mipmappedKey)) {
        return {std::move(cachedMippedView), view.origin(), view.swizzle()};
    }

    auto copy = GrCopyBaseMipMapToView(rContext, view);
    if (!copy) {
        return view;
    }
    // TODO: If we move listeners up from SkImage_Lazy to SkImage_Base then add one here.
    proxyProvider->assignUniqueKeyToProxy(mipmappedKey, copy.asTextureProxy());
    return copy;
}

#endif

GrBackendTexture SkImage_Base::onGetBackendTexture(bool flushPendingGrContextIO,
                                                   GrSurfaceOrigin* origin) const {
    return GrBackendTexture(); // invalid
}

GrDirectContext* SkImage_Base::directContext() const {
#if SK_SUPPORT_GPU
    return GrAsDirectContext(this->context());
#else
    return nullptr;
#endif
}

bool SkImage::readPixels(GrDirectContext* dContext, const SkPixmap& pmap, int srcX, int srcY,
                         CachingHint chint) const {
    return this->readPixels(dContext, pmap.info(), pmap.writable_addr(), pmap.rowBytes(), srcX,
                            srcY, chint);
}

#ifndef SK_IMAGE_READ_PIXELS_DISABLE_LEGACY_API
bool SkImage::readPixels(const SkPixmap& pmap, int srcX, int srcY, CachingHint chint) const {
    auto dContext = as_IB(this)->directContext();
    return this->readPixels(dContext, pmap, srcX, srcY, chint);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeFromBitmap(const SkBitmap& bm) {
    if (!bm.pixelRef()) {
        return nullptr;
    }

    return SkMakeImageFromRasterBitmap(bm, kIfMutable_SkCopyPixelsMode);
}

bool SkImage::asLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode ) const {
    // Context TODO: Elevate GrDirectContext requirement to public API.
    auto dContext = as_IB(this)->directContext();
    return as_IB(this)->onAsLegacyBitmap(dContext, bitmap);
}

bool SkImage_Base::onAsLegacyBitmap(GrDirectContext* dContext, SkBitmap* bitmap) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = fInfo.makeColorType(kN32_SkColorType).makeColorSpace(nullptr);
    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    if (!this->readPixels(dContext, bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(),
                          0, 0)) {
        bitmap->reset();
        return false;
    }

    bitmap->setImmutable();
    return true;
}

sk_sp<SkImage> SkImage::MakeFromPicture(sk_sp<SkPicture> picture, const SkISize& dimensions,
                                        const SkMatrix* matrix, const SkPaint* paint,
                                        BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    return MakeFromGenerator(SkImageGenerator::MakeFromPicture(dimensions, std::move(picture),
                                                               matrix, paint, bitDepth,
                                                               std::move(colorSpace)));
}

sk_sp<SkImage> SkImage::makeWithFilter(GrRecordingContext* rContext, const SkImageFilter* filter,
                                       const SkIRect& subset, const SkIRect& clipBounds,
                                       SkIRect* outSubset, SkIPoint* offset) const {

    if (!filter || !outSubset || !offset || !this->bounds().contains(subset)) {
        return nullptr;
    }
    sk_sp<SkSpecialImage> srcSpecialImage;
#if SK_SUPPORT_GPU
    auto myContext = as_IB(this)->context();
    if (myContext && !myContext->priv().matches(rContext)) {
        return nullptr;
    }
    srcSpecialImage = SkSpecialImage::MakeFromImage(rContext, subset,
                                                    sk_ref_sp(const_cast<SkImage*>(this)),
                                                    SkSurfaceProps());
#else
    srcSpecialImage = SkSpecialImage::MakeFromImage(nullptr, subset,
                                                    sk_ref_sp(const_cast<SkImage*>(this)),
                                                    SkSurfaceProps());
#endif
    if (!srcSpecialImage) {
        return nullptr;
    }

    sk_sp<SkImageFilterCache> cache(
        SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize));

    // The filters operate in the local space of the src image, where (0,0) corresponds to the
    // subset's top left corner. But the clip bounds and any crop rects on the filters are in the
    // original coordinate system, so configure the CTM to correct crop rects and explicitly adjust
    // the clip bounds (since it is assumed to already be in image space).
    SkImageFilter_Base::Context context(SkMatrix::Translate(-subset.x(), -subset.y()),
                                        clipBounds.makeOffset(-subset.topLeft()),
                                        cache.get(), fInfo.colorType(), fInfo.colorSpace(),
                                        srcSpecialImage.get());

    sk_sp<SkSpecialImage> result = as_IFB(filter)->filterImage(context).imageAndOffset(offset);
    if (!result) {
        return nullptr;
    }

    // The output image and offset are relative to the subset rectangle, so the offset needs to
    // be shifted to put it in the correct spot with respect to the original coordinate system
    offset->fX += subset.x();
    offset->fY += subset.y();

    // Final clip against the exact clipBounds (the clip provided in the context gets adjusted
    // to account for pixel-moving filters so doesn't always exactly match when finished). The
    // clipBounds are translated into the clippedDstRect coordinate space, including the
    // result->subset() ensures that the result's image pixel origin does not affect results.
    SkIRect dstRect = result->subset();
    SkIRect clippedDstRect = dstRect;
    if (!clippedDstRect.intersect(clipBounds.makeOffset(result->subset().topLeft() - *offset))) {
        return nullptr;
    }

    // Adjust the geometric offset if the top-left corner moved as well
    offset->fX += (clippedDstRect.x() - dstRect.x());
    offset->fY += (clippedDstRect.y() - dstRect.y());
    *outSubset = clippedDstRect;
    return result->asImage();
}

bool SkImage::isLazyGenerated() const {
    return as_IB(this)->onIsLazyGenerated();
}

bool SkImage::isAlphaOnly() const { return SkColorTypeIsAlphaOnly(fInfo.colorType()); }

sk_sp<SkImage> SkImage::makeColorSpace(sk_sp<SkColorSpace> target, GrDirectContext* direct) const {
    return this->makeColorTypeAndColorSpace(this->colorType(), std::move(target), direct);
}

sk_sp<SkImage> SkImage::makeColorTypeAndColorSpace(SkColorType targetColorType,
                                                   sk_sp<SkColorSpace> targetColorSpace,
                                                   GrDirectContext* dContext) const {
    if (kUnknown_SkColorType == targetColorType || !targetColorSpace) {
        return nullptr;
    }

#if SK_SUPPORT_GPU
    auto myContext = as_IB(this)->context();
    // This check is also performed in the subclass, but we do it here for the short-circuit below.
    if (myContext && !myContext->priv().matches(dContext)) {
        return nullptr;
    }
#endif

    SkColorType colorType = this->colorType();
    SkColorSpace* colorSpace = this->colorSpace();
    if (!colorSpace) {
        colorSpace = sk_srgb_singleton();
    }
    if (colorType == targetColorType &&
        (SkColorSpace::Equals(colorSpace, targetColorSpace.get()) || this->isAlphaOnly())) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }

    return as_IB(this)->onMakeColorTypeAndColorSpace(targetColorType,
                                                     std::move(targetColorSpace), dContext);
}

sk_sp<SkImage> SkImage::reinterpretColorSpace(sk_sp<SkColorSpace> target) const {
    if (!target) {
        return nullptr;
    }

    // No need to create a new image if:
    // (1) The color spaces are equal.
    // (2) The color type is kAlpha8.
    SkColorSpace* colorSpace = this->colorSpace();
    if (!colorSpace) {
        colorSpace = sk_srgb_singleton();
    }
    if (SkColorSpace::Equals(colorSpace, target.get()) || this->isAlphaOnly()) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }

    return as_IB(this)->onReinterpretColorSpace(std::move(target));
}

sk_sp<SkImage> SkImage::makeNonTextureImage() const {
    if (!this->isTextureBacked()) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }
    return this->makeRasterImage();
}

sk_sp<SkImage> SkImage::makeRasterImage(CachingHint chint) const {
    SkPixmap pm;
    if (this->peekPixels(&pm)) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }

    const size_t rowBytes = fInfo.minRowBytes();
    size_t size = fInfo.computeByteSize(rowBytes);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return nullptr;
    }

    // Context TODO: Elevate GrDirectContext requirement to public API.
    auto dContext = as_IB(this)->directContext();
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    pm = {fInfo.makeColorSpace(nullptr), data->writable_data(), fInfo.minRowBytes()};
    if (!this->readPixels(dContext, pm, 0, 0, chint)) {
        return nullptr;
    }

    return SkImage::MakeRasterData(fInfo, std::move(data), rowBytes);
}

//////////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU

sk_sp<SkImage> SkImage::MakeFromTexture(GrRecordingContext*,
                                        const GrBackendTexture&, GrSurfaceOrigin,
                                        SkColorType, SkAlphaType, sk_sp<SkColorSpace>,
                                        TextureReleaseProc, ReleaseContext) {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromCompressedTexture(GrRecordingContext*,
                                                  const GrBackendTexture&,
                                                  GrSurfaceOrigin,
                                                  SkAlphaType,
                                                  sk_sp<SkColorSpace>,
                                                  TextureReleaseProc,
                                                  ReleaseContext) {
    return nullptr;
}

bool SkImage::MakeBackendTextureFromSkImage(GrDirectContext*,
                                            sk_sp<SkImage>,
                                            GrBackendTexture*,
                                            BackendTextureReleaseProc*) {
    return false;
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrRecordingContext*,
                                               const GrBackendTexture&, GrSurfaceOrigin,
                                               SkColorType, SkAlphaType,
                                               sk_sp<SkColorSpace>) {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromYUVAPixmaps(GrRecordingContext* context,
                                            const SkYUVAPixmaps& pixmaps,
                                            GrMipMapped buildMips,
                                            bool limitToMaxTextureSize,
                                            sk_sp<SkColorSpace> imageColorSpace) {
    return nullptr;
}

sk_sp<SkImage> SkImage::makeTextureImage(GrDirectContext*, GrMipmapped, SkBudgeted) const {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakePromiseTexture(sk_sp<GrContextThreadSafeProxy>,
                                           const GrBackendFormat&,
                                           SkISize,
                                           GrMipmapped,
                                           GrSurfaceOrigin,
                                           SkColorType,
                                           SkAlphaType,
                                           sk_sp<SkColorSpace>,
                                           PromiseImageTextureFulfillProc,
                                           PromiseImageTextureReleaseProc,
                                           PromiseImageTextureContext) {
    return nullptr;
}

sk_sp<SkImage> SkImage::MakePromiseYUVATexture(sk_sp<GrContextThreadSafeProxy>,
                                               const GrYUVABackendTextureInfo&,
                                               sk_sp<SkColorSpace>,
                                               PromiseImageTextureFulfillProc,
                                               PromiseImageTextureReleaseProc,
                                               PromiseImageTextureContext[]) {
    return nullptr;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_pinAsTexture(const SkImage* image, GrRecordingContext* rContext) {
    SkASSERT(image);
    SkASSERT(rContext);
    return as_IB(image)->onPinAsTexture(rContext);
}

void SkImage_unpinAsTexture(const SkImage* image, GrRecordingContext* rContext) {
    SkASSERT(image);
    SkASSERT(rContext);
    as_IB(image)->onUnpinAsTexture(rContext);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkMipmapBuilder::SkMipmapBuilder(const SkImageInfo& info) {
    fMM = sk_sp<SkMipmap>(SkMipmap::Build({info, nullptr, 0}, nullptr, false));
}

SkMipmapBuilder::~SkMipmapBuilder() {}

int SkMipmapBuilder::countLevels() const {
    return fMM ? fMM->countLevels() : 0;
}

SkPixmap SkMipmapBuilder::level(int index) const {
    SkPixmap pm;

    SkMipmap::Level level;
    if (fMM && fMM->getLevel(index, &level)) {
        pm = level.fPixmap;
    }
    return pm;
}

bool SkImage::hasMipmaps() const { return as_IB(this)->onHasMipmaps(); }

sk_sp<SkImage> SkImage::withMipmaps(sk_sp<SkMipmap> mips) const {
    if (mips == nullptr || mips->validForRootLevel(this->imageInfo())) {
        if (auto result = as_IB(this)->onMakeWithMipmaps(std::move(mips))) {
            return result;
        }
    }
    return sk_ref_sp((const_cast<SkImage*>(this)));
}

sk_sp<SkImage> SkImage::withDefaultMipmaps() const {
    return this->withMipmaps(nullptr);
}

sk_sp<SkImage> SkMipmapBuilder::attachTo(const SkImage* src) {
    return src->withMipmaps(fMM);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/core/SkReadBuffer.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkWriteBuffer.h"

SkSamplingOptions SkSamplingPriv::FromFQ(SkLegacyFQ fq, SkMediumAs behavior) {
    switch (fq) {
        case kHigh_SkLegacyFQ:
            return SkSamplingOptions(SkCubicResampler{1/3.0f, 1/3.0f});
        case kMedium_SkLegacyFQ:
            return SkSamplingOptions(SkFilterMode::kLinear,
                                      behavior == kNearest_SkMediumAs ? SkMipmapMode::kNearest
                                                                      : SkMipmapMode::kLinear);
        case kLow_SkLegacyFQ:
            return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        case kNone_SkLegacyFQ:
            break;
    }
    return SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
}

SkSamplingOptions SkSamplingPriv::Read(SkReadBuffer& buffer) {
    if (buffer.readBool()) {
        SkScalar B = buffer.readScalar(),
                 C = buffer.readScalar();
        return SkSamplingOptions({B,C});
    } else {
        auto filter = buffer.read32LE<SkFilterMode>(SkFilterMode::kLinear);
        auto mipmap = buffer.read32LE<SkMipmapMode>(SkMipmapMode::kLinear);
        return SkSamplingOptions(filter, mipmap);
    }
}

void SkSamplingPriv::Write(SkWriteBuffer& buffer, const SkSamplingOptions& sampling) {
    buffer.writeBool(sampling.useCubic);
    if (sampling.useCubic) {
        buffer.writeScalar(sampling.cubic.B);
        buffer.writeScalar(sampling.cubic.C);
    } else {
        buffer.writeUInt((unsigned)sampling.filter);
        buffer.writeUInt((unsigned)sampling.mipmap);
    }
}
