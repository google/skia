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
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

#if defined(GRAPHITE_TEST_UTILS)
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

size_t Image::textureSize() const {
    if (!fTextureProxyView.proxy()) {
        return 0;
    }

    if (!fTextureProxyView.proxy()->texture()) {
        return fTextureProxyView.proxy()->uninstantiatedGpuMemorySize();
    }

    return fTextureProxyView.proxy()->texture()->gpuMemorySize();
}

sk_sp<SkImage> Image::onMakeSubset(Recorder* recorder,
                                   const SkIRect& subset,
                                   RequiredProperties requiredProps) const {
    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());

    // optimization : return self if the subset == our bounds and requirements met
    if (bounds == subset && (!requiredProps.fMipmapped || this->hasMipmaps())) {
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

sk_sp<SkImage> Image::makeTextureImage(Recorder* recorder, RequiredProperties requiredProps) const {
    if (!requiredProps.fMipmapped || this->hasMipmaps()) {
        return sk_ref_sp(this);
    }

    // The copied image is not considered budgeted because this is a client-invoked API and they
    // will own the image.
    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    return this->copyImage(recorder,
                           bounds,
                           Budgeted::kNo,
                           requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo,
                           SkBackingFit::kExact);
}

sk_sp<Image> Image::copyImage(Recorder* recorder,
                              const SkIRect& subset,
                              Budgeted budgeted,
                              Mipmapped mipmapped,
                              SkBackingFit backingFit) const {
    SkASSERT(!(mipmapped == Mipmapped::kYes && backingFit == SkBackingFit::kApprox));
    const TextureProxyView& srcView = this->textureProxyView();
    if (!srcView) {
        return nullptr;
    }

    // Attempt copying as a blit first.
    const SkColorInfo& colorInfo = this->imageInfo().colorInfo();
    this->notifyInUse(recorder);
    TextureProxyView copiedView = TextureProxyView::Copy(recorder,
                                                         colorInfo,
                                                         srcView,
                                                         subset,
                                                         budgeted,
                                                         mipmapped,
                                                         backingFit);
    if (copiedView) {
        return sk_make_sp<Image>(std::move(copiedView), colorInfo);
    }

    // Perform the copy as a draw; since the proxy has been wrapped in an Image, it should be
    // texturable.
    SkASSERT(recorder->priv().caps()->isTexturable(srcView.proxy()->textureInfo()));
    return CopyAsDraw(recorder, this, subset, budgeted, mipmapped, backingFit);
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    sk_sp<Image> view = sk_make_sp<Image>(fTextureProxyView,
                                          this->imageInfo().colorInfo()
                                                           .makeColorSpace(std::move(newCS)));
    // The new Image object shares the same texture proxy, so it should also share linked Devices
    view->linkDevices(this);
    return view;
}

sk_sp<SkImage> Image::makeColorTypeAndColorSpace(Recorder* recorder,
                                                 SkColorType targetCT,
                                                 sk_sp<SkColorSpace> targetCS,
                                                 RequiredProperties requiredProps) const {
    SkAlphaType at = (this->alphaType() == kOpaque_SkAlphaType) ? kPremul_SkAlphaType
                                                                : this->alphaType();

    SkImageInfo ii = SkImageInfo::Make(this->dimensions(), targetCT, at, std::move(targetCS));

    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(recorder, ii, mm);
    if (!s) {
        return nullptr;
    }

    s->getCanvas()->drawImage(this, 0, 0);
    return SkSurfaces::AsImage(s);
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;
using SkImages::GraphitePromiseImageFulfillProc;
using SkImages::GraphitePromiseTextureReleaseProc;

sk_sp<TextureProxy> Image::MakePromiseImageLazyProxy(
        const Caps* caps,
        SkISize dimensions,
        TextureInfo textureInfo,
        Volatile isVolatile,
        GraphitePromiseImageFulfillProc fulfillProc,
        sk_sp<skgpu::RefCntedCallback> releaseHelper,
        GraphitePromiseTextureReleaseProc textureReleaseProc) {
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(releaseHelper);

    if (!fulfillProc) {
        return nullptr;
    }

    /**
     * This class is the lazy instantiation callback for promise images. It manages calling the
     * client's Fulfill, ImageRelease, and TextureRelease procs.
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(GraphitePromiseImageFulfillProc fulfillProc,
                                       sk_sp<skgpu::RefCntedCallback> releaseHelper,
                                       GraphitePromiseTextureReleaseProc textureReleaseProc)
                : fFulfillProc(fulfillProc)
                , fReleaseHelper(std::move(releaseHelper))
                , fTextureReleaseProc(textureReleaseProc) {
        }
        PromiseLazyInstantiateCallback(PromiseLazyInstantiateCallback&&) = default;
        PromiseLazyInstantiateCallback(const PromiseLazyInstantiateCallback&) {
            // Because we get wrapped in std::function we must be copyable. But we should never
            // be copied.
            SkASSERT(false);
        }
        PromiseLazyInstantiateCallback& operator=(PromiseLazyInstantiateCallback&&) = default;
        PromiseLazyInstantiateCallback& operator=(const PromiseLazyInstantiateCallback&) {
            SkASSERT(false);
            return *this;
        }

        sk_sp<Texture> operator()(ResourceProvider* resourceProvider) {

            auto [ backendTexture, textureReleaseCtx ] = fFulfillProc(fReleaseHelper->context());
            if (!backendTexture.isValid()) {
                SKGPU_LOG_W("FulFill Proc failed");
                return nullptr;
            }

            sk_sp<RefCntedCallback> textureReleaseCB = RefCntedCallback::Make(fTextureReleaseProc,
                                                                              textureReleaseCtx);

            sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);
            if (!texture) {
                SKGPU_LOG_W("Texture creation failed");
                return nullptr;
            }

            texture->setReleaseCallback(std::move(textureReleaseCB));
            return texture;
        }

    private:
        GraphitePromiseImageFulfillProc fFulfillProc;
        sk_sp<skgpu::RefCntedCallback> fReleaseHelper;
        GraphitePromiseTextureReleaseProc fTextureReleaseProc;

    } callback(fulfillProc, std::move(releaseHelper), textureReleaseProc);

    return TextureProxy::MakeLazy(caps,
                                  dimensions,
                                  textureInfo,
                                  skgpu::Budgeted::kNo,  // This is destined for a user's SkImage
                                  isVolatile,
                                  std::move(callback));
}

#if defined(GRAPHITE_TEST_UTILS)
bool Image::onReadPixelsGraphite(Recorder* recorder,
                                 const SkPixmap& dst,
                                 int srcX,
                                 int srcY) const {
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
