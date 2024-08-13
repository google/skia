/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/task/CopyTask.h"

#if defined(GPU_TEST_UTILS)
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#endif

namespace skgpu::graphite {

// Graphite does not cache based on the image's unique ID so always request a new one.
Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : Image_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {}

Image::~Image() = default;

sk_sp<Image> Image::WrapDevice(sk_sp<Device> device) {
    TextureProxyView proxy = device->readSurfaceView();
    if (!proxy) {
        return nullptr;
    }
    // NOTE: If the device was created with an approx backing fit, its SkImageInfo reports the
    // logical dimensions, but its proxy has the approximate fit. These larger dimensions are
    // propagated to the SkImageInfo of this image view.
    sk_sp<Image> image = sk_make_sp<Image>(std::move(proxy),
                                           device->imageInfo().colorInfo());
    image->linkDevice(std::move(device));
    return image;
}

sk_sp<Image> Image::Copy(Recorder* recorder,
                         const TextureProxyView& srcView,
                         const SkColorInfo& srcColorInfo,
                         const SkIRect& subset,
                         Budgeted budgeted,
                         Mipmapped mipmapped,
                         SkBackingFit backingFit,
                         std::string_view label) {
    SkASSERT(!(mipmapped == Mipmapped::kYes && backingFit == SkBackingFit::kApprox));
    if (!srcView) {
        return nullptr;
    }

    SkASSERT(srcView.proxy()->isFullyLazy() ||
             SkIRect::MakeSize(srcView.proxy()->dimensions()).contains(subset));

    if (!recorder->priv().caps()->supportsReadPixels(srcView.proxy()->textureInfo())) {
        if (!recorder->priv().caps()->isTexturable(srcView.proxy()->textureInfo())) {
            // The texture is not blittable nor texturable so copying cannot be done.
            return nullptr;
        }
        // Copy-as-draw
        sk_sp<Image> srcImage(new Image(srcView, srcColorInfo));
        return CopyAsDraw(recorder, srcImage.get(), subset, srcColorInfo,
                          budgeted, mipmapped, backingFit, std::move(label));
    }


    skgpu::graphite::TextureInfo textureInfo =
            recorder->priv().caps()->getTextureInfoForSampledCopy(srcView.proxy()->textureInfo(),
                                                                  mipmapped);

    sk_sp<TextureProxy> dst = TextureProxy::Make(
            recorder->priv().caps(),
            recorder->priv().resourceProvider(),
            backingFit == SkBackingFit::kApprox ? GetApproxSize(subset.size()) : subset.size(),
            textureInfo,
            std::move(label),
            budgeted);
    if (!dst) {
        return nullptr;
    }

    auto copyTask = CopyTextureToTextureTask::Make(srcView.refProxy(), subset, dst, {0, 0});
    if (!copyTask) {
        return nullptr;
    }

    recorder->priv().add(std::move(copyTask));

    if (mipmapped == Mipmapped::kYes) {
        if (!GenerateMipmaps(recorder, dst, srcColorInfo)) {
            SKGPU_LOG_W("Image::Copy failed to generate mipmaps");
            return nullptr;
        }
    }

    return sk_sp<Image>(new Image({std::move(dst), srcView.swizzle()}, srcColorInfo));
}

size_t Image::textureSize() const {
    if (!fTextureProxyView.proxy()) {
        return 0;
    }

    if (!fTextureProxyView.proxy()->texture()) {
        return fTextureProxyView.proxy()->uninstantiatedGpuMemorySize();
    }

    return fTextureProxyView.proxy()->texture()->gpuMemorySize();
}

sk_sp<Image> Image::copyImage(Recorder* recorder,
                              const SkIRect& subset,
                              Budgeted budgeted,
                              Mipmapped mipmapped,
                              SkBackingFit backingFit,
                              std::string_view label) const {
    this->notifyInUse(recorder, /*drawContext=*/nullptr);
    return Image::Copy(recorder, fTextureProxyView, this->imageInfo().colorInfo(),
                       subset, budgeted, mipmapped, backingFit, std::move(label));
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    sk_sp<Image> view = sk_make_sp<Image>(fTextureProxyView,
                                          this->imageInfo().colorInfo()
                                                           .makeColorSpace(std::move(newCS)));
    // The new Image object shares the same texture proxy, so it should also share linked Devices
    view->linkDevices(this);
    return view;
}

#if defined(GPU_TEST_UTILS)
bool Image::readPixelsGraphite(Recorder* recorder, const SkPixmap& dst, int srcX, int srcY) const {
    if (Context* context = recorder->priv().context()) {
        // Add all previous commands generated to the command buffer.
        // If the client snaps later they'll only get post-read commands in their Recording,
        // but since they're doing a readPixels in the middle that shouldn't be unexpected.
        std::unique_ptr<Recording> recording = recorder->snap();
        if (!recording) {
            return false;
        }
        InsertRecordingInfo info;
        info.fRecording = recording.get();
        if (!context->insertRecording(info)) {
            return false;
        }
        return context->priv().readPixels(dst,
                                          fTextureProxyView.proxy(),
                                          this->imageInfo(),
                                          srcX,
                                          srcY);
    }
    return false;
}
#endif

} // namespace skgpu::graphite
