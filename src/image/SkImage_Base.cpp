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
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkBitmapCache.h"
#include "src/image/SkRescaleAndReadPixels.h"

#if defined(SK_GANESH)
#include "include/gpu/GrRecordingContext.h"
#include "include/private/gpu/ganesh/GrImageContext.h"

#endif

#if defined(SK_GRAPHITE)
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#endif

#include <atomic>

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

GrDirectContext* SkImage_Base::directContext() const {
#if defined(SK_GANESH)
    return GrAsDirectContext(this->context());
#else
    return nullptr;
#endif
}
