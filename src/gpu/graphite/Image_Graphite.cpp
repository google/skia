/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureUtils.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#endif

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

bool Image::testingOnly_ReadPixels(Context* context,
                                   Recorder* recorder,
                                   const SkImageInfo& dstInfo,
                                   void* dstPixels,
                                   size_t dstRowBytes,
                                   int srcX,
                                   int srcY) {
    return ReadPixelsHelper([recorder]() {
                                recorder->priv().flushTrackedDevices();
                            },
                            context,
                            recorder,
                            fTextureProxyView.proxy(),
                            dstInfo,
                            dstPixels,
                            dstRowBytes,
                            srcX,
                            srcY);
}

sk_sp<SkImage> Image::onMakeColorTypeAndColorSpace(SkColorType,
                                                   sk_sp<SkColorSpace>,
                                                   GrDirectContext*) const {
    return nullptr;
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace>) const {
    return nullptr;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> Image::onAsFragmentProcessor(
        GrRecordingContext*,
        SkSamplingOptions,
        const SkTileMode[2],
        const SkMatrix&,
        const SkRect* subset,
        const SkRect* domain) const {
    return nullptr;
}
#endif

sk_sp<SkImage> Image::onMakeTextureImage(Recorder*, RequiredImageProperties requiredProps) const {
    SkASSERT(requiredProps.fMipmapped == Mipmapped::kYes && !this->hasMipmaps());
    // TODO: copy the base layer into a new image that has mip levels. For now we just return
    // the un-mipmapped version and allow the sampling to be downgraded to linear
    SKGPU_LOG_W("Graphite does not yet allow explicit mipmap level addition");
    return sk_ref_sp(this);
}

} // namespace skgpu::graphite

sk_sp<SkImage> SkImage::makeTextureImage(skgpu::graphite::Recorder* recorder,
                                         RequiredImageProperties requiredProps) const {
    using namespace skgpu::graphite;

    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        requiredProps.fMipmapped = Mipmapped::kNo;
    }

    if (as_IB(this)->isGraphiteBacked()) {
        if (requiredProps.fMipmapped == Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }
    return as_IB(this)->onMakeTextureImage(recorder, requiredProps);
}
