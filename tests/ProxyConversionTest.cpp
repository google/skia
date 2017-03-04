/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrRenderTargetProxy.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"

static sk_sp<GrSurfaceProxy> make_wrapped_FBO0(GrResourceProvider* provider,
                                               skiatest::Reporter* reporter,
                                               const GrSurfaceDesc& desc) {
    GrBackendRenderTargetDesc backendDesc;
    backendDesc.fWidth = desc.fWidth;
    backendDesc.fHeight = desc.fHeight;
    backendDesc.fConfig = desc.fConfig;
    backendDesc.fOrigin = desc.fOrigin;
    backendDesc.fSampleCnt = desc.fSampleCnt;
    backendDesc.fStencilBits = 8;
    backendDesc.fRenderTargetHandle = 0;

    sk_sp<GrRenderTarget> defaultFBO(provider->wrapBackendRenderTarget(backendDesc));
    SkASSERT(!defaultFBO->asTexture());

    return GrSurfaceProxy::MakeWrapped(std::move(defaultFBO));
}

static sk_sp<GrSurfaceProxy> make_wrapped_offscreen_rt(GrResourceProvider* provider,
                                                       skiatest::Reporter* reporter,
                                                       const GrSurfaceDesc& desc,
                                                       SkBudgeted budgeted) {
    SkASSERT(kRenderTarget_GrSurfaceFlag == desc.fFlags);

    sk_sp<GrTexture> tex(provider->createTexture(desc, budgeted));

    return GrSurfaceProxy::MakeWrapped(std::move(tex));
}

static sk_sp<GrSurfaceProxy> make_wrapped_texture(GrResourceProvider* provider,
                                                  const GrSurfaceDesc& desc,
                                                  SkBudgeted budgeted) {
    sk_sp<GrTexture> tex(provider->createTexture(desc, budgeted));

    return GrSurfaceProxy::MakeWrapped(std::move(tex));
}

// Test converting between RenderTargetProxies and TextureProxies for wrapped
// Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedProxyConversionTest, reporter, ctxInfo) {
    GrResourceProvider* provider = ctxInfo.grContext()->resourceProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (kOpenGL_GrBackend == ctxInfo.backend()) {
        // External on-screen render target.
        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_FBO0(provider, reporter, desc));

        // RenderTarget-only
        GrRenderTargetProxy* rtProxy = sProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, !rtProxy->asTextureProxy());
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }

    {
        // Internal offscreen render target.
        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_offscreen_rt(provider,
                                                               reporter, desc,
                                                               SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrRenderTargetProxy* rtProxy = sProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }

    {
        // Internal offscreen render target - but through GrTextureProxy
        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_texture(provider,  desc, SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrTextureProxy* tProxy = sProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        GrRenderTargetProxy* rtProxy = tProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
    }

    {
        desc.fFlags = kNone_GrSurfaceFlags; // force no-RT

        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_texture(provider,  desc, SkBudgeted::kYes));

        // Texture-only
        GrTextureProxy* tProxy = sProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}

// Test converting between RenderTargetProxies and TextureProxies for deferred
// Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DefferredProxyConversionTest, reporter, ctxInfo) {
    const GrCaps& caps = *ctxInfo.grContext()->caps();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->resourceProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    {
        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(resourceProvider, caps, desc,
                                                                 SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrRenderTargetProxy* rtProxy = proxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }
    
    {
        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(resourceProvider, caps, desc,
                                                                 SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes));

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

        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(resourceProvider, caps, desc,
                                                                 SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes));

        // Texture-only
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
#endif
