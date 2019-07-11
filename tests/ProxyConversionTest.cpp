/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"

static sk_sp<GrSurfaceProxy> make_wrapped_rt(GrProxyProvider* provider,
                                             GrGpu* gpu,
                                             skiatest::Reporter* reporter,
                                             const GrSurfaceDesc& desc,
                                             GrSurfaceOrigin origin) {
    // We don't currently have a way of making MSAA backend render targets.
    SkASSERT(1 == desc.fSampleCnt);
    auto ct = GrPixelConfigToColorType(desc.fConfig);
    auto backendRT = gpu->createTestingOnlyBackendRenderTarget(desc.fWidth, desc.fHeight, ct);
    return provider->wrapBackendRenderTarget(backendRT, origin, nullptr, nullptr);
}

void clean_up_wrapped_rt(GrGpu* gpu, sk_sp<GrSurfaceProxy> proxy) {
    SkASSERT(proxy->unique());
    SkASSERT(proxy->peekRenderTarget());
    GrBackendRenderTarget rt = proxy->peekRenderTarget()->getBackendRenderTarget();
    proxy.reset();
    gpu->deleteTestingOnlyBackendRenderTarget(rt);
}

static sk_sp<GrSurfaceProxy> make_offscreen_rt(GrProxyProvider* provider,
                                               const GrSurfaceDesc& desc,
                                               GrSurfaceOrigin origin) {
    SkASSERT(kRenderTarget_GrSurfaceFlag == desc.fFlags);

    return provider->testingOnly_createInstantiatedProxy(desc, origin, SkBackingFit::kExact,
                                                         SkBudgeted::kYes);
}

static sk_sp<GrSurfaceProxy> make_texture(GrProxyProvider* provider,
                                          const GrSurfaceDesc& desc,
                                          GrSurfaceOrigin origin) {
    return provider->testingOnly_createInstantiatedProxy(desc, origin, SkBackingFit::kExact,
                                                         SkBudgeted::kYes);
}

// Test converting between RenderTargetProxies and TextureProxies for preinstantiated Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PreinstantiatedProxyConversionTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrGpu* gpu = ctxInfo.grContext()->priv().getGpu();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    {
        // External on-screen render target.
        sk_sp<GrSurfaceProxy> sProxy(
                make_wrapped_rt(proxyProvider, gpu, reporter, desc, kBottomLeft_GrSurfaceOrigin));
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
        sk_sp<GrSurfaceProxy> sProxy(
                make_offscreen_rt(proxyProvider, desc, kBottomLeft_GrSurfaceOrigin));
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
                make_texture(proxyProvider, desc, kBottomLeft_GrSurfaceOrigin));
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
        desc.fFlags = kNone_GrSurfaceFlags; // force no-RT

        sk_sp<GrSurfaceProxy> sProxy(
                make_texture(proxyProvider, desc, kBottomLeft_GrSurfaceOrigin));
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
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DefferredProxyConversionTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    const GrBackendFormat format =
            ctxInfo.grContext()->priv().caps()->getBackendFormatFromColorType(
                    GrColorType::kRGBA_8888);
    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                format, desc, kBottomLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);

        // Both RenderTarget and Texture
        GrRenderTargetProxy* rtProxy = proxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }

    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                format, desc, kBottomLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);

        // Both RenderTarget and Texture - but via GrTextureProxy
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        GrRenderTargetProxy* rtProxy = tProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
    }

    {
        desc.fFlags = kNone_GrSurfaceFlags; // force no-RT

        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                format, desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);
        // Texture-only
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
