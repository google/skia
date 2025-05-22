/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Image_YUVA_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

Image_Base::Image_Base(const SkImageInfo& info, uint32_t uniqueID)
    : SkImage_Base(info, uniqueID) {}

Image_Base::~Image_Base() = default;

void Image_Base::linkDevices(const Image_Base* other) {
    SkASSERT(other);

    SkAutoSpinlock lock{other->fDeviceLinkLock};
    for (const auto& device : other->fLinkedDevices) {
        this->linkDevice(device);
    }
}

void Image_Base::linkDevice(sk_sp<Device> device) {
    // Technically this lock isn't needed since this is only called before the Image is returned to
    // user code that could expose it to multiple threads. But this quiets threading warnings and
    // should be uncontested.
    SkAutoSpinlock lock{fDeviceLinkLock};
    fLinkedDevices.push_back(std::move(device));
}

void Image_Base::notifyInUse(Recorder* recorder, DrawContext* drawContext) const {
    SkASSERT(recorder);

    // The ref counts stored on each linked device are thread safe, but the Image's sk_sp's that
    // track the refs its responsible for are *not* thread safe. Use a spin lock since the majority
    // of device-linked images will be used only on the Recorder's thread. Since it should be
    // uncontended, the empty check is also done inside the lock vs. a double-checked locking
    // pattern that is non-trivial to ensure correctness in C++.
    SkAutoSpinlock lock{fDeviceLinkLock};

    if (!fLinkedDevices.empty()) {
        int emptyCount = 0;
        for (sk_sp<Device>& device : fLinkedDevices) {
            if (!device) {
                emptyCount++; // Already unlinked but array isn't empty yet
            } else {
                if (device->isScratchDevice()) {
                    sk_sp<Task> deviceDrawTask = device->lastDrawTask();
                    if (deviceDrawTask) {
                        // Increment the pending read count for the device's target
                        recorder->priv().addPendingRead(device->target());
                        if (drawContext) {
                            // Add a reference to the device's drawTask to `drawContext` if that's
                            // provided.
                            drawContext->recordDependency(std::move(deviceDrawTask));
                        } else {
                            // If there's no `drawContext` this notify represents a copy, so for
                            // now append the task to the root task list since that is where the
                            // subsequent copy task will go as well.
                            recorder->priv().add(std::move(deviceDrawTask));
                        }
                    } else {
                        // If there's no draw task yet, the device is being drawn into a child
                        // scratch device (backdrop filter or init-from-prev layer), and the child
                        // will later on be drawn back into the device's `drawContext`. In this case
                        // `device` should already have performed an internal flush and have no
                        // pending work, and not yet be marked immutable. The correct action at this
                        // point in time is to do nothing: the final task order in the device's
                        // DrawTask will be pre-notified tasks into the device's target, then the
                        // child's DrawTask when it's drawn back into `device`, and then any post
                        // tasks that further modify the `device`'s target.
                        SkASSERT(device->recorder() && device->recorder() == recorder);
                    }

                    // Scratch devices are often already marked immutable, but they are also the
                    // way in which Image finds the last snapped DrawTask so we don't unlink
                    // scratch devices. The scratch image view will be short-lived as well, or the
                    // device will transition to a non-scratch device in a future Recording and then
                    // it will be unlinked then.
                } else {
                    // Automatic flushing of image views only happens when mixing reads and writes
                    // on the originating Recorder. Draws of the view on another Recorder will
                    // always see the texture content dependent on how Recordings are inserted.
                    if (device->recorder() == recorder) {
                        // Non-scratch devices push their tasks to the root task list to maintain
                        // an order consistent with the client-triggering actions. Because of this,
                        // there's no need to add references to the `drawContext` that the device
                        // is being drawn into.
                        device->flushPendingWorkToRecorder();
                    }
                    if (!device->recorder() || device->unique()) {
                        // The device will not record any more commands that modify the texture, so
                        // the image doesn't need to be linked
                        device.reset();
                        emptyCount++;
                    }
                }
            }
        }

        if (emptyCount == fLinkedDevices.size()) {
            fLinkedDevices.clear();
        }
    }
}

bool Image_Base::isDynamic() const {
    SkAutoSpinlock lock{fDeviceLinkLock};
    int emptyCount = 0;
    if (!fLinkedDevices.empty()) {
        for (sk_sp<Device>& device : fLinkedDevices) {
            if (!device || !device->recorder() || device->unique()) {
                device.reset();
                emptyCount++;
            }
        }
        if (emptyCount == fLinkedDevices.size()) {
            fLinkedDevices.clear();
            emptyCount = 0;
        }
    }

    return emptyCount > 0;
}

sk_sp<Image> Image_Base::copyImage(Recorder* recorder,
                                   const SkIRect& subset,
                                   Budgeted budgeted,
                                   Mipmapped mipmapped,
                                   SkBackingFit backingFit,
                                   std::string_view label) const {
    return CopyAsDraw(recorder, this, subset, this->imageInfo().colorInfo(),
                      budgeted, mipmapped, backingFit, std::move(label));
}

namespace {

TextureProxy* get_base_proxy_for_label(const Image_Base* baseImage) {
    if (baseImage->type() == SkImage_Base::Type::kGraphite) {
        const Image* img = static_cast<const Image*>(baseImage);
        return img->textureProxyView().proxy();
    }
    SkASSERT(baseImage->type() == SkImage_Base::Type::kGraphiteYUVA);
    // We will end up flattening to RGBA for a YUVA image when we get a subset. We just grab
    // the label off of the first channel's proxy and use that to be the stand in label.
    const Image_YUVA* img = static_cast<const Image_YUVA*>(baseImage);
    return img->proxyView(0).proxy();
}

} // anonymous namespace

sk_sp<SkImage> Image_Base::onMakeSubset(Recorder* recorder,
                                        const SkIRect& subset,
                                        RequiredProperties requiredProps) const {
    // optimization : return self if the subset == our bounds and requirements met and the image's
    // texture is immutable
    if (this->bounds() == subset &&
        (!requiredProps.fMipmapped || this->hasMipmaps()) &&
        !this->isDynamic()) {
        return sk_ref_sp(this);
    }

    TextureProxy* proxy = get_base_proxy_for_label(this);
    SkASSERT(proxy);
    std::string label = proxy->label();
    if (label.empty()) {
        label = "ImageSubsetTexture";
    } else {
        label += "_Subset";
    }

    // The copied image is not considered budgeted because this is a client-invoked API and they
    // will own the image.
    return this->copyImage(recorder,
                           subset,
                           Budgeted::kNo,
                           requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                           SkBackingFit::kExact,
                           label);
}

sk_sp<SkSurface> Image_Base::onMakeSurface(SkRecorder* recorder, const SkImageInfo& info) const {
    auto gRecorder = AsGraphiteRecorder(recorder);
    if (!gRecorder) {
        return nullptr;
    }
    return SkSurfaces::RenderTarget(gRecorder, info);
}

sk_sp<SkImage> Image_Base::makeColorTypeAndColorSpace(SkRecorder* recorder,
                                                      SkColorType targetCT,
                                                      sk_sp<SkColorSpace> targetCS,
                                                      RequiredProperties requiredProps) const {
    auto gRecorder = AsGraphiteRecorder(recorder);
    if (!gRecorder) {
        return nullptr;
    }

    SkColorInfo dstColorInfo{targetCT, this->alphaType(), std::move(targetCS)};
    // optimization : return self if there's no color type/space change and the image's texture
    // is immutable
    if (this->imageInfo().colorInfo() == dstColorInfo && !this->isDynamic()) {
        return sk_ref_sp(this);
    }

    TextureProxy* proxy = get_base_proxy_for_label(this);
    SkASSERT(proxy);
    std::string label = proxy->label();
    if (label.empty()) {
        label = "ImageMakeCTandCSTexture";
    } else {
        label += "_CTandCSConversion";
    }

    // Use CopyAsDraw directly to perform the color space changes. The copied image is not
    // considered budgeted because this is a client-invoked API and they will own the image.
    return CopyAsDraw(gRecorder,
                      this,
                      this->bounds(),
                      dstColorInfo,
                      Budgeted::kNo,
                      requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                      SkBackingFit::kExact,
                      label);
}

// Ganesh APIs are no-ops

sk_sp<SkImage> Image_Base::onMakeSubset(GrDirectContext*, const SkIRect&) const {
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
    SKGPU_LOG_W("Cannot use Ganesh async API with Graphite-backed image, use API on Context");
    callback(context, nullptr);
}

void Image_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                   bool readAlpha,
                                                   sk_sp<SkColorSpace> dstColorSpace,
                                                   const SkIRect srcRect,
                                                   const SkISize dstSize,
                                                   RescaleGamma rescaleGamma,
                                                   RescaleMode rescaleMode,
                                                   ReadPixelsCallback callback,
                                                   ReadPixelsContext context) const {
    SKGPU_LOG_W("Cannot use Ganesh async API with Graphite-backed image, use API on Context");
    callback(context, nullptr);
}

} // namespace skgpu::graphite
