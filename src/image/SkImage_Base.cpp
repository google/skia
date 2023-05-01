/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Base.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkSpecialImage.h"
#include "src/image/SkRescaleAndReadPixels.h"

#if defined(SK_GRAPHITE)
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#endif

#include <atomic>
#include <utility>

class SkImageFilter;

SkImage_Base::SkImage_Base(const SkImageInfo& info, uint32_t uniqueID)
        : SkImage(info, uniqueID), fAddedToRasterCache(false) {}

SkImage_Base::~SkImage_Base() {
    if (fAddedToRasterCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

void SkImage_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                               SkIRect origSrcRect,
                                               RescaleGamma rescaleGamma,
                                               RescaleMode rescaleMode,
                                               ReadPixelsCallback callback,
                                               ReadPixelsContext context) const {
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

bool SkImage_Base::onAsLegacyBitmap(GrDirectContext* dContext, SkBitmap* bitmap) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = fInfo.makeColorType(kN32_SkColorType).makeColorSpace(nullptr);
    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    if (!this->readPixels(
                dContext, bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0)) {
        bitmap->reset();
        return false;
    }

    bitmap->setImmutable();
    return true;
}

sk_sp<SkImage> SkImage_Base::makeSubset(GrDirectContext* direct, const SkIRect& subset) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    // optimization : return self if the subset == our bounds
    if (bounds == subset) {
        return sk_ref_sp(const_cast<SkImage_Base*>(this));
    }

    return as_IB(this)->onMakeSubset(direct, subset);
}

sk_sp<SkImage> SkImage_Base::makeWithFilter(GrRecordingContext*,
                                            const SkImageFilter* filter,
                                            const SkIRect& subset,
                                            const SkIRect& clipBounds,
                                            SkIRect* outSubset,
                                            SkIPoint* offset) const {
    if (!filter || !outSubset || !offset || !this->bounds().contains(subset)) {
        return nullptr;
    }

    auto srcSpecialImage = SkSpecialImage::MakeFromImage(
            nullptr, subset, sk_ref_sp(const_cast<SkImage_Base*>(this)), SkSurfaceProps());
    if (!srcSpecialImage) {
        return nullptr;
    }

    sk_sp<SkImageFilterCache> cache(
            SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize));

    // The filters operate in the local space of the src image, where (0,0) corresponds to the
    // subset's top left corner. But the clip bounds and any crop rects on the filters are in the
    // original coordinate system, so configure the CTM to correct crop rects and explicitly adjust
    // the clip bounds (since it is assumed to already be in image space).
    // TODO: Once all image filters support it, we can just use the subset's top left corner as
    // the source FilterResult's origin.
    skif::ContextInfo ctxInfo = {
            skif::Mapping(SkMatrix::Translate(-subset.x(), -subset.y())),
            skif::LayerSpace<SkIRect>(clipBounds.makeOffset(-subset.topLeft())),
            skif::FilterResult(srcSpecialImage),
            fInfo.colorType(),
            fInfo.colorSpace(),
            /*fSurfaceProps=*/{},
            cache.get()};
    skif::Context context = skif::Context::MakeRaster(ctxInfo);

    return this->filterSpecialImage(
            context, as_IFB(filter), srcSpecialImage.get(), subset, clipBounds, outSubset, offset);
}

sk_sp<SkImage> SkImage_Base::filterSpecialImage(skif::Context context,
                                                const SkImageFilter_Base* filter,
                                                const SkSpecialImage* specialImage,
                                                const SkIRect& subset,
                                                const SkIRect& clipBounds,
                                                SkIRect* outSubset,
                                                SkIPoint* offset) const {
    sk_sp<SkSpecialImage> result = filter->filterImage(context).imageAndOffset(context, offset);
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

void SkImage_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                     sk_sp<SkColorSpace> dstColorSpace,
                                                     SkIRect srcRect,
                                                     SkISize dstSize,
                                                     RescaleGamma,
                                                     RescaleMode,
                                                     ReadPixelsCallback callback,
                                                     ReadPixelsContext context) const {
    // TODO: Call non-YUV asyncRescaleAndReadPixels and then make our callback convert to YUV and
    // call client's callback.
    callback(context, nullptr);
}

sk_sp<SkImage> SkImage_Base::makeColorTypeAndColorSpace(GrDirectContext* dContext,
                                                        SkColorType targetColorType,
                                                        sk_sp<SkColorSpace> targetCS) const {
    if (kUnknown_SkColorType == targetColorType || !targetCS) {
        return nullptr;
    }

    SkColorType colorType = this->colorType();
    SkColorSpace* colorSpace = this->colorSpace();
    if (!colorSpace) {
        colorSpace = sk_srgb_singleton();
    }
    if (colorType == targetColorType &&
        (SkColorSpace::Equals(colorSpace, targetCS.get()) || this->isAlphaOnly())) {
        return sk_ref_sp(const_cast<SkImage_Base*>(this));
    }

    return this->onMakeColorTypeAndColorSpace(targetColorType, std::move(targetCS), dContext);
}

#if defined(SK_GRAPHITE)
std::tuple<skgpu::graphite::TextureProxyView, SkColorType> SkImage_Base::asView(
        skgpu::graphite::Recorder* recorder,
        skgpu::Mipmapped mipmapped) const {
    if (!recorder) {
        return {};
    }

    if (!as_IB(this)->isGraphiteBacked()) {
        return {};
    }
    // TODO(b/238756380): YUVA not supported yet
    if (as_IB(this)->isYUVA()) {
        return {};
    }

    auto image = reinterpret_cast<const skgpu::graphite::Image*>(this);

    if (this->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (mipmapped == skgpu::Mipmapped::kYes &&
        image->textureProxyView().proxy()->mipmapped() != skgpu::Mipmapped::kYes) {
        SKGPU_LOG_W("Graphite does not auto-generate mipmap levels");
        return {};
    }

    SkColorType ct = this->colorType();
    return { image->textureProxyView(), ct };
}

sk_sp<SkImage> SkImage::makeColorSpace(sk_sp<SkColorSpace> targetColorSpace,
                                       skgpu::graphite::Recorder* recorder,
                                       RequiredImageProperties requiredProps) const {
    return this->makeColorTypeAndColorSpace(this->colorType(), std::move(targetColorSpace),
                                            recorder, requiredProps);
}

sk_sp<SkImage> SkImage::makeColorTypeAndColorSpace(SkColorType targetColorType,
                                                   sk_sp<SkColorSpace> targetColorSpace,
                                                   skgpu::graphite::Recorder* recorder,
                                                   RequiredImageProperties requiredProps) const {
    if (kUnknown_SkColorType == targetColorType || !targetColorSpace) {
        return nullptr;
    }

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
                                                     std::move(targetColorSpace),
                                                     recorder,
                                                     requiredProps);
}

sk_sp<SkImage> SkImage_Base::makeTextureImage(skgpu::graphite::Recorder* recorder,
                                              RequiredImageProperties requiredProps) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        requiredProps.fMipmapped = skgpu::Mipmapped::kNo;
    }

    return as_IB(this)->onMakeTextureImage(recorder, requiredProps);
}

#endif // SK_GRAPHITE
