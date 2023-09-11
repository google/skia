/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <utility>

struct GrContextOptions;

static sk_sp<GrSurfaceProxy> make_wrapped_rt(GrProxyProvider* provider,
                                             GrGpu* gpu,
                                             skiatest::Reporter* reporter,
                                             const SkISize& size,
                                             GrColorType colorType) {
    auto backendRT = gpu->createTestingOnlyBackendRenderTarget(size, colorType);
    return provider->wrapBackendRenderTarget(backendRT, nullptr);
}

void clean_up_wrapped_rt(GrGpu* gpu, sk_sp<GrSurfaceProxy> proxy) {
    SkASSERT(proxy->unique());
    SkASSERT(proxy->peekRenderTarget());
    GrBackendRenderTarget rt = proxy->peekRenderTarget()->getBackendRenderTarget();
    proxy.reset();
    gpu->deleteTestingOnlyBackendRenderTarget(rt);
}

static sk_sp<GrSurfaceProxy> make_offscreen_rt(GrProxyProvider* provider,
                                               SkISize dimensions,
                                               GrColorType colorType) {
    return provider->testingOnly_createInstantiatedProxy(dimensions,
                                                         colorType,
                                                         GrRenderable::kYes,
                                                         1,
                                                         SkBackingFit::kExact,
                                                         skgpu::Budgeted::kYes,
                                                         GrProtected::kNo);
}

static sk_sp<GrSurfaceProxy> make_texture(GrProxyProvider* provider,
                                          SkISize dimensions,
                                          GrColorType colorType,
                                          GrRenderable renderable) {
    return provider->testingOnly_createInstantiatedProxy(dimensions,
                                                         colorType,
                                                         renderable,
                                                         1,
                                                         SkBackingFit::kExact,
                                                         skgpu::Budgeted::kYes,
                                                         GrProtected::kNo);
}

// Test converting between RenderTargetProxies and TextureProxies for preinstantiated Proxies
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PreinstantiatedProxyConversionTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrGpu* gpu = context->priv().getGpu();

    static constexpr auto kSize = SkISize::Make(64, 64);
    static constexpr auto kColorType = GrColorType::kRGBA_8888;

    {
        // External on-screen render target.
        sk_sp<GrSurfaceProxy> sProxy(
                make_wrapped_rt(proxyProvider, gpu, reporter, kSize, kColorType));
        if (sProxy) {
            // RenderTarget-only
            GrRenderTargetProxy* rtProxy = sProxy->asRenderTargetProxy();
            REPORTER_ASSERT(reporter, rtProxy);
            REPORTER_ASSERT(reporter, !rtProxy->asTextureProxy());
            REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
            clean_up_wrapped_rt(gpu, std::move(sProxy));
        }
    }

    {
        // Internal offscreen render target.
        sk_sp<GrSurfaceProxy> sProxy(make_offscreen_rt(proxyProvider, kSize, kColorType));
        if (sProxy) {
            // Both RenderTarget and Texture
            GrRenderTargetProxy* rtProxy = sProxy->asRenderTargetProxy();
            REPORTER_ASSERT(reporter, rtProxy);
            GrTextureProxy* tProxy = rtProxy->asTextureProxy();
            REPORTER_ASSERT(reporter, tProxy);
            REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy);
            REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
        }
    }

    {
        // Internal offscreen render target - but through GrTextureProxy
        sk_sp<GrSurfaceProxy> sProxy(
                make_texture(proxyProvider, kSize, kColorType, GrRenderable::kYes));
        if (sProxy) {
            // Both RenderTarget and Texture
            GrTextureProxy* tProxy = sProxy->asTextureProxy();
            REPORTER_ASSERT(reporter, tProxy);
            GrRenderTargetProxy* rtProxy = tProxy->asRenderTargetProxy();
            REPORTER_ASSERT(reporter, rtProxy);
            REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == tProxy);
            REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        }
    }

    {
        // force no-RT
        sk_sp<GrSurfaceProxy> sProxy(
                make_texture(proxyProvider, kSize, kColorType, GrRenderable::kNo));
        if (sProxy) {
            // Texture-only
            GrTextureProxy* tProxy = sProxy->asTextureProxy();
            REPORTER_ASSERT(reporter, tProxy);
            REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
            REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
        }
    }
}

// Test converting between RenderTargetProxies and TextureProxies for deferred
// Proxies
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DefferredProxyConversionTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    static constexpr SkISize kDims = {64, 64};

    const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                 GrRenderable::kYes);
    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format,
                                                                 kDims,
                                                                 GrRenderable::kYes,
                                                                 1,
                                                                 skgpu::Mipmapped::kNo,
                                                                 SkBackingFit::kApprox,
                                                                 skgpu::Budgeted::kYes,
                                                                 GrProtected::kNo,
                                                                 /*label=*/{});

        // Both RenderTarget and Texture
        GrRenderTargetProxy* rtProxy = proxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }

    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format,
                                                                 kDims,
                                                                 GrRenderable::kYes,
                                                                 1,
                                                                 skgpu::Mipmapped::kNo,
                                                                 SkBackingFit::kApprox,
                                                                 skgpu::Budgeted::kYes,
                                                                 GrProtected::kNo,
                                                                 /*label=*/{});

        // Both RenderTarget and Texture - but via GrTextureProxy
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        GrRenderTargetProxy* rtProxy = tProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
    }

    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format,
                                                                 kDims,
                                                                 GrRenderable::kNo,
                                                                 1,
                                                                 skgpu::Mipmapped::kNo,
                                                                 SkBackingFit::kApprox,
                                                                 skgpu::Budgeted::kYes,
                                                                 GrProtected::kNo,
                                                                 /*label=*/{});
        // Texture-only
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
