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
#include "src/core/SkColorSpacePriv.h"
#include "src/image/SkRescaleAndReadPixels.h"

#include <atomic>
#include <utility>

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

    return this->onMakeSubset(direct, subset);
}

sk_sp<SkImage> SkImage_Base::makeSubset(skgpu::graphite::Recorder* recorder,
                                        const SkIRect& subset,
                                        RequiredProperties requiredProps) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    return this->onMakeSubset(recorder, subset, requiredProps);
}

void SkImage_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                     bool readAlpha,
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

sk_sp<SkImage> SkImage_Base::makeColorSpace(GrDirectContext* direct,
                                            sk_sp<SkColorSpace> target) const {
    return this->makeColorTypeAndColorSpace(direct, this->colorType(), std::move(target));
}

sk_sp<SkImage> SkImage_Base::makeColorSpace(SkRecorder* recorder,
                                            sk_sp<SkColorSpace> target,
                                            RequiredProperties props) const {
    return this->makeColorTypeAndColorSpace(recorder, this->colorType(), std::move(target), props);
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

sk_sp<SkImage> SkImage_Base::makeColorTypeAndColorSpace(SkRecorder* recorder,
                                                        SkColorType ct,
                                                        sk_sp<SkColorSpace> cs,
                                                        RequiredProperties) const {
    // Default to the ganesh version which should be backend agnostic if this
    // image is, for example, a raster backed image. The graphite subclass overrides
    // this method and things work correctly.
    return this->makeColorTypeAndColorSpace(nullptr, ct, std::move(cs));
}
