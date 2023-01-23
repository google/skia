/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/RefCntedCallback.h"
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

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : Image_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

sk_sp<SkImage> Image::onMakeSubset(const SkIRect& subset,
                                   Recorder* recorder,
                                   RequiredImageProperties requiredProps) const {
    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());

    // optimization : return self if the subset == our bounds and requirements met
    if (bounds == subset && (requiredProps.fMipmapped == Mipmapped::kNo || this->hasMipmaps())) {
        const SkImage* image = this;
        return sk_ref_sp(const_cast<SkImage*>(image));
    }

    return this->copyImage(subset, recorder, requiredProps);
}

sk_sp<SkImage> Image::onMakeTextureImage(Recorder* recorder,
                                         RequiredImageProperties requiredProps) const {
    if (requiredProps.fMipmapped == Mipmapped::kNo || this->hasMipmaps()) {
        const SkImage* image = this;
        return sk_ref_sp(const_cast<SkImage*>(image));
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    return this->copyImage(bounds, recorder, requiredProps);
}

sk_sp<SkImage> Image::copyImage(const SkIRect& subset,
                                Recorder* recorder,
                                RequiredImageProperties requiredProps) const {
    TextureProxyView srcView = this->textureProxyView();
    if (!srcView) {
        return nullptr;
    }

    TextureProxyView copiedView = TextureProxyView::Copy(recorder,
                                                         this->imageInfo().colorInfo(),
                                                         srcView,
                                                         subset,
                                                         requiredProps.fMipmapped);
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

sk_sp<SkImage> Image::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                   sk_sp<SkColorSpace> targetCS,
                                                   Recorder* recorder,
                                                   RequiredImageProperties requiredProps) const {
    SkAlphaType at = (this->alphaType() == kOpaque_SkAlphaType) ? kPremul_SkAlphaType
                                                                : this->alphaType();

    SkImageInfo ii = SkImageInfo::Make(this->dimensions(), targetCT, at, std::move(targetCS));

    sk_sp<SkSurface> s = SkSurface::MakeGraphite(recorder, ii, requiredProps.fMipmapped);
    if (!s) {
        return nullptr;
    }

    s->getCanvas()->drawImage(this, 0, 0);

    return s->asImage();
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;

namespace {

bool validate_backend_texture(const Caps* caps,
                              const BackendTexture& texture,
                              const SkColorInfo& info) {
    if (!texture.isValid() ||
        texture.dimensions().width() <= 0 ||
        texture.dimensions().height() <= 0) {
        return false;
    }

    if (!SkColorInfoIsValid(info)) {
        return false;
    }

    if (!caps->isTexturable(texture.info())) {
        return false;
    }

    return caps->areColorTypeAndTextureInfoCompatible(info.colorType(), texture.info());
}

} // anonymous namespace

sk_sp<TextureProxy> Image::MakePromiseImageLazyProxy(
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

    return TextureProxy::MakeLazy(dimensions,
                                  textureInfo,
                                  skgpu::Budgeted::kNo,  // This is destined for a user's SkImage
                                  isVolatile,
                                  std::move(callback));
}

sk_sp<SkImage> SkImage::MakeGraphitePromiseTexture(
        Recorder* recorder,
        SkISize dimensions,
        const TextureInfo& textureInfo,
        const SkColorInfo& colorInfo,
        Volatile isVolatile,
        GraphitePromiseImageFulfillProc fulfillProc,
        GraphitePromiseImageReleaseProc imageReleaseProc,
        GraphitePromiseTextureReleaseProc textureReleaseProc,
        GraphitePromiseImageContext imageContext) {

    // Our contract is that we will always call the _image_ release proc even on failure.
    // We use the helper to convey the imageContext, so we need to ensure Make doesn't fail.
    imageReleaseProc = imageReleaseProc ? imageReleaseProc : [](void*) {};
    auto releaseHelper = skgpu::RefCntedCallback::Make(imageReleaseProc, imageContext);

    if (!recorder) {
        SKGPU_LOG_W("Null Recorder");
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkImageInfo info = SkImageInfo::Make(dimensions, colorInfo);
    if (!SkImageInfoIsValid(info)) {
        SKGPU_LOG_W("Invalid SkImageInfo");
        return nullptr;
    }

    if (!caps->areColorTypeAndTextureInfoCompatible(colorInfo.colorType(), textureInfo)) {
        SKGPU_LOG_W("Incompatible SkColorType and TextureInfo");
        return nullptr;
    }

    sk_sp<TextureProxy> proxy = Image::MakePromiseImageLazyProxy(dimensions,
                                                                 textureInfo,
                                                                 isVolatile,
                                                                 fulfillProc,
                                                                 std::move(releaseHelper),
                                                                 textureReleaseProc);
    if (!proxy) {
        return nullptr;
    }

    skgpu::Swizzle swizzle = caps->getReadSwizzle(colorInfo.colorType(), textureInfo);
    TextureProxyView view(std::move(proxy), swizzle);
    return sk_make_sp<Image>(view, colorInfo);
}

sk_sp<SkImage> SkImage::MakeGraphiteFromBackendTexture(Recorder* recorder,
                                                       const BackendTexture& backendTex,
                                                       SkColorType ct,
                                                       SkAlphaType at,
                                                       sk_sp<SkColorSpace> cs) {
    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkColorInfo info(ct, at, std::move(cs));

    if (!validate_backend_texture(caps, backendTex, info)) {
        return nullptr;
    }

    sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return nullptr;
    }

    sk_sp<TextureProxy> proxy(new TextureProxy(std::move(texture)));

    skgpu::Swizzle swizzle = caps->getReadSwizzle(ct, backendTex.info());
    TextureProxyView view(std::move(proxy), swizzle);
    return sk_make_sp<Image>(view, info);
}
