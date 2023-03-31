/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "src/gpu/graphite/Log.h"

namespace skgpu::graphite {

sk_sp<SkImage> Image_Base::onMakeSubset(const SkIRect&, GrDirectContext*) const {
    SKGPU_LOG_W("Cannot convert Graphite-backed image to Ganesh");
    return nullptr;
}

sk_sp<SkImage> Image_Base::onMakeColorTypeAndColorSpace(SkColorType,
                                                        sk_sp<SkColorSpace>,
                                                        GrDirectContext*) const {
    SKGPU_LOG_W("Cannot convert Graphite-backed image to Ganesh");
    return nullptr;
}

void Image_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                             SkIRect srcRect,
                                             RescaleGamma rescaleGamma,
                                             RescaleMode rescaleMode,
                                             ReadPixelsCallback callback,
                                             ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

void Image_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                   sk_sp<SkColorSpace> dstColorSpace,
                                                   const SkIRect srcRect,
                                                   const SkISize dstSize,
                                                   RescaleGamma rescaleGamma,
                                                   RescaleMode rescaleMode,
                                                   ReadPixelsCallback callback,
                                                   ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;

sk_sp<SkImage> SkImage::makeSubset(const SkIRect& subset,
                                   skgpu::graphite::Recorder* recorder,
                                   RequiredImageProperties requiredProps) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    return as_IB(this)->onMakeSubset(subset, recorder, requiredProps);
}
