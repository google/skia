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
#include "src/gpu/graphite/Image_Graphite.h"
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

Image_YUVA::Image_YUVA(YUVATextureProxies proxies,
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
                     kNeedNewImageUniqueID)
        , fYUVAProxies(std::move(proxies)) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAProxies.isValid());
}

Image_YUVA::~Image_YUVA() = default;

sk_sp<Image_YUVA> Image_YUVA::WrapImages(const Caps* caps,
                                         const SkYUVAInfo& yuvaInfo,
                                         SkSpan<const sk_sp<SkImage>> images,
                                         sk_sp<SkColorSpace> imageColorSpace) {
    int numPlanes = yuvaInfo.numPlanes();
    if ((size_t) numPlanes > images.size()) {
        return nullptr;
    }
    TextureProxyView textureProxyViews[SkYUVAInfo::kMaxPlanes];
    for (int plane = 0; plane < numPlanes; ++plane) {
        if (as_IB(images[plane])->type() != SkImage_Base::Type::kGraphite) {
            return nullptr;
        }

        textureProxyViews[plane] = static_cast<Image*>(images[plane].get())->textureProxyView();
        // YUVATextureProxies expects to sample from the red channel for single-channel textures, so
        // reset the swizzle for alpha-only textures to compensate for that
        if (images[plane]->isAlphaOnly()) {
            textureProxyViews[plane] = textureProxyViews[plane].makeSwizzle(skgpu::Swizzle("aaaa"));
        }
    }
    YUVATextureProxies yuvaProxies(caps, yuvaInfo, SkSpan<TextureProxyView>(textureProxyViews));
    SkASSERT(yuvaProxies.isValid());
    sk_sp<Image_YUVA> view = sk_make_sp<Image_YUVA>(std::move(yuvaProxies),
                                                    std::move(imageColorSpace));
    // Unlike the other factories, this YUVA image shares the texture proxies with each plane Image,
    // so if those are linked to Devices, it must inherit those same links.
    for (int plane = 0; plane < numPlanes; ++plane) {
        SkASSERT(as_IB(images[plane])->isGraphiteBacked());
        view->linkDevices(static_cast<Image_Base*>(images[plane].get()));
    }
    return view;
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
    sk_sp<Image_YUVA> view = sk_make_sp<Image_YUVA>(fYUVAProxies, std::move(newCS));
    // The new Image object shares the same texture planes, so it should also share linked Devices
    view->linkDevices(this);
    return view;
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
