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
#include "include/private/SkLog.h"
#include "include/private/SkPixelStorage.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Image_YUVA_Graphite.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/task/Task.h"

namespace skgpu::graphite {

Image_Base::Image_Base(const SkImageInfo& info, uint32_t uniqueID, sk_sp<SkPixelStorage> storage)
    : SkImage_Base(info, uniqueID, std::move(storage)) {}

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

void Image_Base::notifyInUse(Recorder* recorder,
                             DrawContext* drawContext,
                             bool unlinkDevices) const {
    SkASSERT(recorder);
    SkASSERT(!drawContext || !unlinkDevices); // unlinkDevices can't be used with a DrawContext

    // The ref counts stored on each linked device are thread safe, but the Image's sk_sp's that
    // track the refs its responsible for are *not* thread safe. Use a spin lock since the majority
    // of device-linked images will be used only on the Recorder's thread. Since it should be
    // uncontended, the empty check is also done inside the lock vs. a double-checked locking
    // pattern that is non-trivial to ensure correctness in C++.
    SkAutoSpinlock lock{fDeviceLinkLock};

    if (!fLinkedDevices.empty()) {
        int emptyCount = 0;
        for (sk_sp<Device>& device : fLinkedDevices) {
            if (!device || device->notifyInUse(recorder, drawContext) || unlinkDevices) {
                // Already unlinked or notifyInUse() signals the device doesn't need to be linked
                // anymore. reset() is a no-op if device is already holding null. If we are force
                // unlinking a device, it should already be immutable (which do not return true from
                // notifyInUse for scratch devices, hence the forcing).
                SkASSERT(!device || !unlinkDevices || !device->recorder());
                device.reset();
                emptyCount++;
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

namespace {

// Uses the label of the first proxy, since cases where we chain to make a new image will flatten
// any YUVA image.
std::string get_chained_label(const Image_Base* baseImage,
                              const char* forEmpty,
                              const char* forConcat) {
    SkASSERT(baseImage && !baseImage->textureProxyViews().empty());
    TextureProxy* proxy = baseImage->textureProxyViews()[0].proxy();
    SkASSERT(proxy);
    std::string label = proxy->label();
    if (label.empty()) {
        label = forEmpty;
    } else {
        label += forConcat;
    }
    return label;
}

} // anonymous namespace

sk_sp<Image_Base> Image_Base::makeNonBudgeted(Recorder* recorder) {
    // First iterate the proxies held by the image and see if they are instantiated or need to be
    // updated to Budgeted::kNo.
    bool needsInstantiation = false;
    for (TextureProxyView& view : this->textureProxyViews()) {
        if (view.proxy()->isInstantiated()) {
            // At this point, the properties of the TextureProxy are locked in.
            const Texture* texture = view.proxy()->texture();
            if (texture->budgeted() != Budgeted::kNo || texture->shareable() != Shareable::kNo) {
                // Not compatible but instantiated, so make a copy that is non-budgeted.
                return this->copyImage(recorder,
                                       this->bounds(),
                                       Budgeted::kNo,
                                       this->hasMipmaps() ? Mipmapped::kYes : Mipmapped::kNo,
                                       SkBackingFit::kExact,
                                       get_chained_label(this, "NonBudgeted", "_NonBudgeted"));
            }
            // else this proxy is already consistent with the contract, so continue to any other
            // planes that might need to be updated.
        } else {
            needsInstantiation = true;
        }
    }

    if (needsInstantiation) {
        // There are presumably tasks (either in the root task list already or on a linked Device)
        // that will initialize this image's proxy. Since the tasks reference the existing
        // TextureProxy, we can't create a new proxy that is non-budgeted. Instead we modify it
        // directly and then unlink this Image from its devices.
        for (TextureProxyView& view : this->textureProxyViews()) {
            if (!view.proxy()->isInstantiated()) {
                view.proxy()->setBudgeted(Budgeted::kNo);
                if (!view.proxy()->instantiate(recorder->priv().resourceProvider())) {
                    return nullptr;
                }
            } else {
                SkASSERT(view.proxy()->texture()->budgeted() == Budgeted::kNo &&
                         view.proxy()->texture()->shareable() == Shareable::kNo);
            }
        }

        this->unlinkDevices(recorder);
    }

    // At this point we already were not dynamic, or we unlinked all our devices, or we should not
    // have reached here and returned a copy instead.
    SkASSERT(!this->isDynamic());
    return sk_ref_sp(this);
}

// For now, CopyAsDraw is called with a nullptr for the drawContext, which causes the draw task
// to be pushed onto the root task list. However, this could be given a drawContext in the future.
sk_sp<Image> Image_Base::copyImage(Recorder* recorder,
                                   const SkIRect& subset,
                                   Budgeted budgeted,
                                   Mipmapped mipmapped,
                                   SkBackingFit backingFit,
                                   std::string_view label) const {
    return CopyAsDraw(recorder,
                      /*drawContext=*/nullptr, this, subset, this->imageInfo().colorInfo(),
                      budgeted, mipmapped, backingFit, label);
}

sk_sp<SkImage> Image_Base::onMakeSubset(SkRecorder* recorder,
                                        const SkIRect& subset,
                                        RequiredProperties requiredProps) const {
    auto gRecorder = AsGraphiteRecorder(recorder);
    if (!gRecorder) {
        return nullptr;
    }
    // optimization : return self if the subset == our bounds and requirements met and the image's
    // texture is immutable
    if (this->bounds() == subset &&
        (!requiredProps.fMipmapped || this->hasMipmaps()) &&
        !this->isDynamic()) {
        return sk_ref_sp(this);
    }

    // The copied image is not considered budgeted because this is a client-invoked API and they
    // will own the image.
    return this->copyImage(gRecorder,
                           subset,
                           Budgeted::kNo,
                           requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                           SkBackingFit::kExact,
                           get_chained_label(this, "ImageSubsetTexture", "_Subset"));
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

    // Use CopyAsDraw directly to perform the color space changes. The copied image is not
    // considered budgeted because this is a client-invoked API and they will own the image.
    return CopyAsDraw(gRecorder,
                      /*drawContext=*/nullptr,
                      this,
                      this->bounds(),
                      dstColorInfo,
                      Budgeted::kNo,
                      requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                      SkBackingFit::kExact,
                      get_chained_label(this, "ImageMakeCTandCSTexture", "_CTandCSConversion"));
}

// Ganesh APIs are no-ops
void Image_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                             SkIRect srcRect,
                                             RescaleGamma rescaleGamma,
                                             RescaleMode rescaleMode,
                                             ReadPixelsCallback callback,
                                             ReadPixelsContext context) const {
    SKIA_LOG_W("Cannot use Ganesh async API with Graphite-backed image, use API on Context");
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
    SKIA_LOG_W("Cannot use Ganesh async API with Graphite-backed image, use API on Context");
    callback(context, nullptr);
}

} // namespace skgpu::graphite
