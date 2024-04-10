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
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/Surface_Graphite.h"

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

sk_sp<Image> Image_Base::CopyAsDraw(Recorder* recorder,
                                    const Image_Base* image,
                                    const SkIRect& subset,
                                    const SkColorInfo& dstColorInfo,
                                    Budgeted budgeted,
                                    Mipmapped mipmapped,
                                    SkBackingFit backingFit) {
    SkImageInfo dstInfo = SkImageInfo::Make(subset.size(),
                                            dstColorInfo.makeAlphaType(kPremul_SkAlphaType));
    // The surface goes out of scope when we return, so it can be scratch, but it may or may
    // not be budgeted depending on how the copied image is used (or returned to the client).
    auto surface = Surface::MakeScratch(recorder,
                                        dstInfo,
                                        budgeted,
                                        mipmapped,
                                        backingFit);
    if (!surface) {
        return nullptr;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawImage(image, -subset.left(), -subset.top(),
                                    SkFilterMode::kNearest, &paint);
    // And the image draw into `surface` is flushed when it goes out of scope
    return surface->asImage();
}

sk_sp<Image> Image_Base::copyImage(Recorder* recorder,
                                   const SkIRect& subset,
                                   Budgeted budgeted,
                                   Mipmapped mipmapped,
                                   SkBackingFit backingFit) const {
    return CopyAsDraw(recorder, this, subset, this->imageInfo().colorInfo(),
                      budgeted, mipmapped, backingFit);
}

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

    // The copied image is not considered budgeted because this is a client-invoked API and they
    // will own the image.
    return this->copyImage(recorder,
                           subset,
                           Budgeted::kNo,
                           requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                           SkBackingFit::kExact);
}

sk_sp<SkImage> Image_Base::makeColorTypeAndColorSpace(Recorder* recorder,
                                                      SkColorType targetCT,
                                                      sk_sp<SkColorSpace> targetCS,
                                                      RequiredProperties requiredProps) const {
    SkColorInfo dstColorInfo{targetCT, this->alphaType(), std::move(targetCS)};
    // optimization : return self if there's no color type/space change and the image's texture
    // is immutable
    if (this->imageInfo().colorInfo() == dstColorInfo && !this->isDynamic()) {
        return sk_ref_sp(this);
    }

    // Use CopyAsDraw directly to perform the color space changes. The copied image is not
    // considered budgeted because this is a client-invoked API and they will own the image.
    return CopyAsDraw(recorder,
                      this,
                      this->bounds(),
                      dstColorInfo,
                      Budgeted::kNo,
                      requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                      SkBackingFit::kExact);
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
