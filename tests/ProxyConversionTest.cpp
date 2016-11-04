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
#include "GrRenderTargetPriv.h"
#include "GrRenderTargetProxy.h"

// TODO: need to test oplist handling
// TODO: need to flesh out asTexProxy & asRTProxy calls
// TODO: test backing here? if not, elsewhere!
// TODO: add check that instantiated IDs match between proxy and resource

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
    REPORTER_ASSERT(reporter, !defaultFBO->renderTargetPriv().maxWindowRectangles());
    SkASSERT(!defaultFBO->asTexture());

    return GrRenderTargetProxy::Make(std::move(defaultFBO));
}

static sk_sp<GrRenderTargetProxy> make_wrapped_offscreen_rt(GrTextureProvider* provider, 
                                                            const GrCaps& caps,
                                                            skiatest::Reporter* reporter,
                                                            const GrSurfaceDesc& desc,
                                                            SkBudgeted budgeted) {
    GrSurfaceDesc localDesc = desc;
    localDesc.fFlags = kRenderTarget_GrSurfaceFlag;

    sk_sp<GrTexture> tex(provider->createTexture(localDesc, budgeted));
    sk_sp<GrRenderTarget> rt(sk_ref_sp(tex->asRenderTarget()));
    REPORTER_ASSERT(reporter, caps.maxWindowRectangles() ==
                                        rt->renderTargetPriv().maxWindowRectangles());

    return GrRenderTargetProxy::Make(rt);
}

static sk_sp<GrTextureProxy> make_wrapped_texture(GrTextureProvider* provider, 
                                                  const GrSurfaceDesc& desc,
                                                  SkBudgeted budgeted) {
    GrSurfaceDesc localDesc = desc;
    localDesc.fSampleCnt = 0;

    sk_sp<GrTexture> tex(provider->createTexture(localDesc, budgeted));
    SkASSERT(!tex->asRenderTarget());

    return GrTextureProxy::Make(tex);
}

// Test converting between RenderTargetProxies and TextureProxies for wrapped
// Proxies
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedProxyConversionTest, reporter, ctxInfo) {
    GrTextureProvider* provider = ctxInfo.grContext()->textureProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    GrSurfaceDesc desc;
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
        sk_sp<GrRenderTargetProxy> rtProxy(make_wrapped_offscreen_rt(provider, caps,
                                                                     reporter, desc,
                                                                     SkBudgeted::kYes));

        // Both RenderTarget and Texture
        GrTextureProxy* tProxy = rtProxy->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);
        REPORTER_ASSERT(reporter, tProxy->asRenderTargetProxy() == rtProxy.get());
        REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy.get());
    }

    {
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

        // Texture-only
        REPORTER_ASSERT(reporter, tProxy->asTextureProxy() == tProxy.get());
        REPORTER_ASSERT(reporter, !tProxy->asRenderTargetProxy());
    }
}
#endif
