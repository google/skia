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
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
    : Image_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

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
        const SkImage* image = this;
        return sk_ref_sp(const_cast<SkImage*>(image));
    }

    return this->copyImage(subset, recorder, requiredProps);
}

sk_sp<SkImage> Image::makeTextureImage(Recorder* recorder, RequiredProperties requiredProps) const {
    if (!requiredProps.fMipmapped || this->hasMipmaps()) {
        const SkImage* image = this;
        return sk_ref_sp(const_cast<SkImage*>(image));
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    return this->copyImage(bounds, recorder, requiredProps);
}

sk_sp<SkImage> Image::copyImage(const SkIRect& subset,
                                Recorder* recorder,
                                RequiredProperties requiredProps) const {
    const TextureProxyView& srcView = this->textureProxyView();
    if (!srcView) {
        return nullptr;
    }

    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    TextureProxyView copiedView = TextureProxyView::Copy(
            recorder, this->imageInfo().colorInfo(), srcView, subset, mm, SkBackingFit::kExact);
    if (!copiedView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(kNeedNewImageUniqueID,
                                  std::move(copiedView),
                                  this->imageInfo().colorInfo()));
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<Image>(kNeedNewImageUniqueID,
                             fTextureProxyView,
                             this->imageInfo().colorInfo().makeColorSpace(std::move(newCS)));
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
