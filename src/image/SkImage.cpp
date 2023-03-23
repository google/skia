/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"

#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkNextID.h"
#include "src/core/SkSpecialImage.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkImageShader.h"

#include <utility>

class SkShader;

#if defined(SK_GANESH)
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#endif

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
                                        ReadPixelsContext context) const {
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
                                              ReadPixelsContext context) const {
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

sk_sp<SkShader> SkImage::makeShader(const SkSamplingOptions& sampling, const SkMatrix& lm) const {
    return SkImageShader::Make(sk_ref_sp(const_cast<SkImage*>(this)),
                               SkTileMode::kClamp, SkTileMode::kClamp,
                               sampling, &lm);
}

sk_sp<SkShader> SkImage::makeShader(const SkSamplingOptions& sampling, const SkMatrix* lm) const {
    return SkImageShader::Make(sk_ref_sp(const_cast<SkImage*>(this)),
                               SkTileMode::kClamp, SkTileMode::kClamp,
                               sampling, lm);
}

sk_sp<SkShader> SkImage::makeShader(SkTileMode tmx, SkTileMode tmy,
                                    const SkSamplingOptions& sampling,
                                    const SkMatrix& lm) const {
    return SkImageShader::Make(sk_ref_sp(const_cast<SkImage*>(this)), tmx, tmy,
                               sampling, &lm);
}

sk_sp<SkShader> SkImage::makeShader(SkTileMode tmx, SkTileMode tmy,
                                    const SkSamplingOptions& sampling,
                                    const SkMatrix* localMatrix) const {
    return SkImageShader::Make(sk_ref_sp(const_cast<SkImage*>(this)), tmx, tmy,
                               sampling, localMatrix);
}

sk_sp<SkShader> SkImage::makeRawShader(SkTileMode tmx, SkTileMode tmy,
                                       const SkSamplingOptions& sampling,
                                       const SkMatrix& lm) const {
    return SkImageShader::MakeRaw(sk_ref_sp(const_cast<SkImage*>(this)), tmx, tmy,
                                  sampling, &lm);
}

sk_sp<SkShader> SkImage::makeRawShader(const SkSamplingOptions& sampling,
                                       const SkMatrix& lm) const {
    return SkImageShader::MakeRaw(sk_ref_sp(const_cast<SkImage*>(this)),
                                  SkTileMode::kClamp, SkTileMode::kClamp,
                                  sampling, &lm);
}

sk_sp<SkShader> SkImage::makeRawShader(const SkSamplingOptions& sampling,
                                       const SkMatrix* localMatrix) const {
    return SkImageShader::MakeRaw(sk_ref_sp(const_cast<SkImage*>(this)),
                                  SkTileMode::kClamp, SkTileMode::kClamp,
                                  sampling, localMatrix);
}

sk_sp<SkShader> SkImage::makeRawShader(SkTileMode tmx, SkTileMode tmy,
                                       const SkSamplingOptions& sampling,
                                       const SkMatrix* localMatrix) const {
    return SkImageShader::MakeRaw(sk_ref_sp(const_cast<SkImage*>(this)), tmx, tmy,
                                  sampling, localMatrix);
}

sk_sp<SkData> SkImage::encodeToData(GrDirectContext* context, SkEncodedImageFormat type,
                                    int quality) const {
    SkBitmap bm;
    if (as_IB(this)->getROPixels(context, &bm)) {
        return SkEncodeBitmap(bm, type, quality);
    }
    return nullptr;
}

sk_sp<SkData> SkImage::encodeToData(GrDirectContext* context) const {
    if (auto encoded = this->refEncodedData()) {
        return encoded;
    }

    return this->encodeToData(context, SkEncodedImageFormat::kPNG, 100);
}

#ifndef SK_IMAGE_READ_PIXELS_DISABLE_LEGACY_API
sk_sp<SkData> SkImage::encodeToData(SkEncodedImageFormat type, int quality) const {
    auto dContext = as_IB(this)->directContext();
    return this->encodeToData(dContext, type, quality);
}

sk_sp<SkData> SkImage::encodeToData() const {
    auto dContext = as_IB(this)->directContext();
    return this->encodeToData(dContext);
}
#endif

sk_sp<SkData> SkImage::refEncodedData() const {
    return sk_sp<SkData>(as_IB(this)->onRefEncoded());
}

sk_sp<SkImage> SkImage::MakeFromEncoded(sk_sp<SkData> encoded,
                                        std::optional<SkAlphaType> alphaType) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    return SkImage::MakeFromGenerator(
            SkImageGenerator::MakeFromEncoded(std::move(encoded), alphaType));
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

#if defined(SK_GANESH)
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

#if defined(SK_GANESH)

bool SkImage::isTextureBacked() const {
    return as_IB(this)->isGaneshBacked() || as_IB(this)->isGraphiteBacked();
}

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

GrSemaphoresSubmitted SkImage::flush(GrDirectContext* dContext,
                                     const GrFlushInfo& flushInfo) const {
    return as_IB(this)->onFlush(dContext, flushInfo);
}

void SkImage::flushAndSubmit(GrDirectContext* dContext) const {
    this->flush(dContext, {});
    dContext->submit();
}

#else

bool SkImage::isTextureBacked() const { return false; }

bool SkImage::isValid(GrRecordingContext* rContext) const {
    if (rContext) {
        return false;
    }
    return as_IB(this)->onIsValid(nullptr);
}

#endif

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

sk_sp<SkImage> SkImage::MakeFromPicture(sk_sp<SkPicture> picture, const SkISize& dimensions,
                                        const SkMatrix* matrix, const SkPaint* paint,
                                        BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace) {
    return SkImage::MakeFromPicture(picture, dimensions, matrix, paint, bitDepth, colorSpace, {});
}

sk_sp<SkImage> SkImage::MakeFromPicture(sk_sp<SkPicture> picture, const SkISize& dimensions,
                                        const SkMatrix* matrix, const SkPaint* paint,
                                        BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                                        SkSurfaceProps props) {
    return MakeFromGenerator(SkImageGenerator::MakeFromPicture(dimensions, std::move(picture),
                                                               matrix, paint, bitDepth,
                                                               std::move(colorSpace), props));
}

sk_sp<SkImage> SkImage::makeWithFilter(GrRecordingContext* rContext, const SkImageFilter* filter,
                                       const SkIRect& subset, const SkIRect& clipBounds,
                                       SkIRect* outSubset, SkIPoint* offset) const {

    if (!filter || !outSubset || !offset || !this->bounds().contains(subset)) {
        return nullptr;
    }
    sk_sp<SkSpecialImage> srcSpecialImage;
#if defined(SK_GANESH)
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

#if defined(SK_GANESH)
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
