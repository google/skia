/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetProxy.h"
#include "GrSurfaceProxy.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"

static sk_sp<GrSurfaceProxy> make_wrapped_rt(GrProxyProvider* provider,
                                             GrGpu* gpu,
                                             skiatest::Reporter* reporter,
                                             const GrSurfaceDesc& desc,
                                             GrSurfaceOrigin origin) {
    // We don't currently have a way of making MSAA backend render targets.
    SkASSERT(1 == desc.fSampleCnt);
    GrSRGBEncoded srgbEncoded;
    auto ct = GrPixelConfigToColorTypeAndEncoding(desc.fConfig, &srgbEncoded);
    auto backendRT = gpu->createTestingOnlyBackendRenderTarget(desc.fWidth, desc.fHeight, ct,
                                                               GrSRGBEncoded::kNo);
    return provider->wrapBackendRenderTarget(backendRT, origin);
}

void clean_up_wrapped_rt(GrGpu* gpu, sk_sp<GrSurfaceProxy> proxy) {
    SkASSERT(proxy->isUnique_debugOnly());
    SkASSERT(proxy->priv().peekRenderTarget());
    GrBackendRenderTarget rt = proxy->priv().peekRenderTarget()->getBackendRenderTarget();
    proxy.reset();
    gpu->deleteTestingOnlyBackendRenderTarget(rt);
}

static sk_sp<GrSurfaceProxy> make_offscreen_rt(GrProxyProvider* provider,
                                               const GrSurfaceDesc& desc,
                                               GrSurfaceOrigin origin) {
    SkASSERT(kRenderTarget_GrSurfaceFlag == desc.fFlags);

    return provider->createInstantiatedProxy(desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);
}

static sk_sp<GrSurfaceProxy> make_texture(GrProxyProvider* provider,
                                          const GrSurfaceDesc& desc,
                                          GrSurfaceOrigin origin) {
    return provider->createInstantiatedProxy(desc, origin, SkBackingFit::kExact, SkBudgeted::kYes);
}

// Test converting between RenderTargetProxies and TextureProxies for preinstantiated Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PreinstantiatedProxyConversionTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();
    GrGpu* gpu = ctxInfo.grContext()->contextPriv().getGpu();

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
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    {
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                desc, kBottomLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);

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
                desc, kBottomLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);

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
                desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);
        // Texture-only
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
