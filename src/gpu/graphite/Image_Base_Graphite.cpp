/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Log.h"

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

void Image_Base::notifyInUse(Recorder* recorder) const {
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
                // Automatic flushing of image views only happens when mixing reads and writes on
                // the originating Recorder. Draws of the view on another Recorder will always see
                // the texture content dependent on how Recordings are inserted.
                if (device->recorder() == recorder) {
                    device->flushPendingWorkToRecorder();
                }
                if (!device->recorder() || device->unique()) {
                    // The device will not record any more commands that modify the texture, so the
                    // image doesn't need to be linked
                    device.reset();
                    emptyCount++;
                }
            }
        }
        if (emptyCount == fLinkedDevices.size()) {
            fLinkedDevices.clear();
        }
    }
}

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
