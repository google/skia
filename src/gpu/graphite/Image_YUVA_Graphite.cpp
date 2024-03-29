/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_YUVA_Graphite.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/YUVABackendTextures.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/shaders/SkImageShader.h"

namespace {
constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;
}

namespace skgpu::graphite {

Image_YUVA::Image_YUVA(uint32_t uniqueID,
                       YUVATextureProxies proxies,
                       sk_sp<SkColorSpace> imageColorSpace)
        : Image_Base(SkImageInfo::Make(proxies.yuvaInfo().dimensions(),
                                       kAssumedColorType,
                                       // If an alpha channel is present we always use kPremul. This
                                       // is because, although the planar data is always un-premul,
                                       // the final interleaved RGBA sample produced in the shader
                                       // is premul (and similar if flattened).
                                       proxies.yuvaInfo().hasAlpha() ? kPremul_SkAlphaType
                                                                     : kOpaque_SkAlphaType,
                                       std::move(imageColorSpace)),
                     uniqueID)
        , fYUVAProxies(std::move(proxies)) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAProxies.isValid());
}

size_t Image_YUVA::textureSize() const {
    size_t size = 0;
    for (int i = 0; i < fYUVAProxies.numPlanes(); ++i) {
        if (fYUVAProxies.proxy(i)->texture()) {
            size += fYUVAProxies.proxy(i)->texture()->gpuMemorySize();
        }
    }
    return size;
}

sk_sp<SkImage> Image_YUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<Image_YUVA>(kNeedNewImageUniqueID, fYUVAProxies, std::move(newCS));
}

sk_sp<SkImage> Image_YUVA::makeTextureImage(Recorder* recorder,
                                            RequiredProperties requiredProps) const {
    // Not clear if we want a flattened image here or not
    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(recorder, this->imageInfo(), mm);
    if (!s) {
        return nullptr;
    }

    s->getCanvas()->drawImage(this, 0, 0);
    return SkSurfaces::AsImage(s);
}

sk_sp<SkImage> Image_YUVA::onMakeSubset(Recorder* recorder,
                                        const SkIRect& subset,
                                        RequiredProperties requiredProps) const {

    SkImageInfo info = this->imageInfo().makeWH(subset.width(), subset.height());
    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(recorder, info, mm);
    if (!s) {
        return nullptr;
    }

    // Translate the subset to the origin of the destination
    SkMatrix m = SkMatrix::Translate(-subset.x(), -subset.y());
    SkPaint p;
    p.setShader(SkImageShader::MakeSubset(sk_ref_sp(this),
                                          SkRect::Make(subset),
                                          SkTileMode::kClamp, SkTileMode::kClamp,
                                          SkSamplingOptions(SkFilterMode::kNearest), &m));
    s->getCanvas()->drawRect(SkRect::Make(info.bounds()), p);

    return SkSurfaces::AsImage(s);
}

sk_sp<SkImage> Image_YUVA::makeColorTypeAndColorSpace(Recorder* recorder,
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

}  // namespace skgpu::graphite

using namespace skgpu::graphite;
using SkImages::GraphitePromiseImageYUVAFulfillProc;
using SkImages::GraphitePromiseTextureContext;
using SkImages::GraphitePromiseTextureReleaseProc;

sk_sp<TextureProxy> Image_YUVA::MakePromiseImageLazyProxy(
        const Caps* caps,
        SkISize dimensions,
        TextureInfo textureInfo,
        Volatile isVolatile,
        GraphitePromiseImageYUVAFulfillProc fulfillProc,
        sk_sp<skgpu::RefCntedCallback> releaseHelper,
        GraphitePromiseTextureContext textureContext,
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
        PromiseLazyInstantiateCallback(GraphitePromiseImageYUVAFulfillProc fulfillProc,
                                       sk_sp<skgpu::RefCntedCallback> releaseHelper,
                                       GraphitePromiseTextureContext textureContext,
                                       GraphitePromiseTextureReleaseProc textureReleaseProc)
                : fFulfillProc(fulfillProc)
                , fReleaseHelper(std::move(releaseHelper))
                , fTextureContext(textureContext)
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

            auto [ backendTexture, textureReleaseCtx ] = fFulfillProc(fTextureContext);
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
        GraphitePromiseImageYUVAFulfillProc fFulfillProc;
        sk_sp<skgpu::RefCntedCallback> fReleaseHelper;
        GraphitePromiseTextureContext  fTextureContext;
        GraphitePromiseTextureReleaseProc fTextureReleaseProc;

    } callback(fulfillProc, std::move(releaseHelper), textureContext, textureReleaseProc);

    return TextureProxy::MakeLazy(caps,
                                  dimensions,
                                  textureInfo,
                                  skgpu::Budgeted::kNo,  // This is destined for a user's SkImage
                                  isVolatile,
                                  std::move(callback));
}
