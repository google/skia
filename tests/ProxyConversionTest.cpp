/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetProxy.h"
#include "GrSurfaceProxy.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"

static sk_sp<GrSurfaceProxy> make_wrapped_FBO0(GrProxyProvider* provider,
                                               skiatest::Reporter* reporter,
                                               const GrSurfaceDesc& desc) {
    GrGLFramebufferInfo fboInfo;
    fboInfo.fFBOID = 0;
    GrBackendRenderTarget backendRT(desc.fWidth, desc.fHeight, desc.fSampleCnt, 8,
                                    desc.fConfig, fboInfo);

#if 0
    sk_sp<GrRenderTarget> defaultFBO(provider->wrapBackendRenderTarget(backendRT));
    SkASSERT(!defaultFBO->asTexture());

    return GrSurfaceProxy::MakeWrapped(std::move(defaultFBO), desc.fOrigin);
#else
    return provider->createWrappedRenderTargetProxy1(backendRT, desc.fOrigin);
#endif
}

static sk_sp<GrSurfaceProxy> make_wrapped_offscreen_rt(GrProxyProvider* provider,
                                                       const GrSurfaceDesc& desc) {
    SkASSERT(kRenderTarget_GrSurfaceFlag == desc.fFlags);

#if 0
    sk_sp<GrTexture> tex(provider->createTexture(desc, budgeted));

    return GrSurfaceProxy::MakeWrapped(std::move(tex), desc.fOrigin);
#else
    return provider->createFoo(desc, SkBackingFit::kExact, SkBudgeted::kYes, 0);
#endif
}

static sk_sp<GrSurfaceProxy> make_wrapped_texture(GrProxyProvider* provider,
                                                  const GrSurfaceDesc& desc) {
#if 0
    sk_sp<GrTexture> tex(provider->createTexture(desc, budgeted));

    return GrSurfaceProxy::MakeWrapped(std::move(tex), desc.fOrigin);
#else
    return provider->createFoo(desc, SkBackingFit::kExact, SkBudgeted::kYes, 0);
#endif
}

// Test converting between RenderTargetProxies and TextureProxies for wrapped
// Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedProxyConversionTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;

    if (kOpenGL_GrBackend == ctxInfo.backend()) {
        // External on-screen render target.
        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_FBO0(proxyProvider, reporter, desc));

        // RenderTarget-only
        GrRenderTargetProxy* rtProxy = sProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, !rtProxy->asTextureProxy());
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }

    {
        // Internal offscreen render target.
        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_offscreen_rt(proxyProvider, desc));

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
        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_texture(proxyProvider, desc));

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

        sk_sp<GrSurfaceProxy> sProxy(make_wrapped_texture(proxyProvider, desc));

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
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    {
#if 0
        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(proxyProvider, desc,
                                                                 SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes));
#else
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(desc, SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes, 0);
#endif

        // Both RenderTarget and Texture
        GrRenderTargetProxy* rtProxy = proxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);
    }

    {
#if 0
        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(proxyProvider, desc,
                                                                 SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes));
#else
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(desc, SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes, 0);
#endif

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
        desc.fOrigin = kTopLeft_GrSurfaceOrigin;

#if 0
        sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeDeferred(proxyProvider, desc,
                                                                 SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes));
#else
        sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(desc, SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes, 0);
#endif
        // Texture-only
        GrTextureProxy* tProxy = proxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy);
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
#endif
