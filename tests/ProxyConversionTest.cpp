/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"
#include "GrRenderTargetProxy.h"

static sk_sp<GrRenderTargetProxy> make_wrapped_FBO0(GrTextureProvider* provider,
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

    return GrRenderTargetProxy::Make(std::move(defaultFBO));
}

static sk_sp<GrRenderTargetProxy> make_wrapped_offscreen_rt(GrTextureProvider* provider, 
                                                            skiatest::Reporter* reporter,
                                                            const GrSurfaceDesc& desc,
                                                            SkBudgeted budgeted) {
    SkASSERT(kRenderTarget_GrSurfaceFlag == desc.fFlags);

    sk_sp<GrTexture> tex(provider->createTexture(desc, budgeted));
    sk_sp<GrRenderTarget> rt(sk_ref_sp(tex->asRenderTarget()));

    return GrRenderTargetProxy::Make(std::move(rt));
}

static sk_sp<GrTextureProxy> make_wrapped_texture(GrTextureProvider* provider, 
                                                  const GrSurfaceDesc& desc,
                                                  SkBudgeted budgeted) {
    sk_sp<GrTexture> tex(provider->createTexture(desc, budgeted));

    return GrTextureProxy::Make(std::move(tex));
}

// Test converting between RenderTargetProxies and TextureProxies for wrapped
// Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedProxyConversionTest, reporter, ctxInfo) {
    GrTextureProvider* provider = ctxInfo.grContext()->textureProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (kOpenGL_GrBackend == ctxInfo.backend()) {
        // External on-screen render target.
        sk_sp<GrRenderTargetProxy> rtProxy(make_wrapped_FBO0(provider, reporter, desc));

        // RenderTarget-only
        REPORTER_ASSERT(reporter, !rtProxy->asTextureProxy());
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy.get());
    }

    {
        // Internal offscreen render target.
        sk_sp<GrRenderTargetProxy> rtProxy(make_wrapped_offscreen_rt(provider,
                                                                     reporter, desc,
                                                                     SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy.get());
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy.get());
    }

    {
        // Internal offscreen render target - but through GrTextureProxy
        sk_sp<GrTextureProxy> tProxy(make_wrapped_texture(provider,  desc, SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrRenderTargetProxy* rtProxy = tProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == tProxy.get());
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy.get());        
    }

    {
        desc.fFlags = kNone_GrSurfaceFlags; // force no-RT

        sk_sp<GrTextureProxy> tProxy(make_wrapped_texture(provider,  desc, SkBudgeted::kYes));

        // Texture-only
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy.get());
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}

// Test converting between RenderTargetProxies and TextureProxies for deferred
// Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DefferredProxyConversionTest, reporter, ctxInfo) {
    GrTextureProvider* provider = ctxInfo.grContext()->textureProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 64;
    desc.fHeight = 64;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    {
        sk_sp<GrRenderTargetProxy> rtProxy(GrRenderTargetProxy::Make(caps, desc,
                                                                     SkBackingFit::kApprox,
                                                                     SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy.get());
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy.get());
    }
    
    {
        sk_sp<GrTextureProxy> tProxy(GrTextureProxy::Make(caps, provider, desc,
                                                          SkBackingFit::kApprox,
                                                          SkBudgeted::kYes));

        // Both RenderTarget and Texture - but via GrTextureProxy
        GrRenderTargetProxy* rtProxy = tProxy->asRenderTargetProxy();
        REPORTER_ASSERT(reporter, rtProxy);
        REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == tProxy.get());
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy.get());
    }

    {
        desc.fFlags = kNone_GrSurfaceFlags; // force no-RT

        sk_sp<GrTextureProxy> tProxy(GrTextureProxy::Make(caps, provider, desc,
                                                          SkBackingFit::kApprox,
                                                          SkBudgeted::kYes));

        // Texture-only
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy.get());
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
#endif
