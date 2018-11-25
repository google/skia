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
#include "GrRenderTargetPriv.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"

// Check that the surface proxy's member vars are set as expected
static void check_surface(skiatest::Reporter* reporter,
                          GrSurfaceProxy* proxy,
                          GrSurfaceOrigin origin,
                          int width, int height,
                          GrPixelConfig config,
                          SkBudgeted budgeted) {
    REPORTER_ASSERT(reporter, proxy->origin() == origin);
    REPORTER_ASSERT(reporter, proxy->width() == width);
    REPORTER_ASSERT(reporter, proxy->height() == height);
    REPORTER_ASSERT(reporter, proxy->config() == config);
    REPORTER_ASSERT(reporter, !proxy->uniqueID().isInvalid());
    REPORTER_ASSERT(reporter, proxy->isBudgeted() == budgeted);
}

static void check_rendertarget(skiatest::Reporter* reporter,
                               const GrCaps& caps,
                               GrResourceProvider* provider,
                               GrRenderTargetProxy* rtProxy,
                               int numSamples,
                               SkBackingFit fit,
                               int expectedMaxWindowRects,
                               bool wasWrapped) {
    REPORTER_ASSERT(reporter, rtProxy->maxWindowRectangles(caps) == expectedMaxWindowRects);
    REPORTER_ASSERT(reporter, rtProxy->numStencilSamples() == numSamples);

    GrSurfaceProxy::UniqueID idBefore = rtProxy->uniqueID();
    REPORTER_ASSERT(reporter, rtProxy->instantiate(provider));
    GrRenderTarget* rt = rtProxy->priv().peekRenderTarget();

    REPORTER_ASSERT(reporter, rtProxy->uniqueID() == idBefore);
    if (wasWrapped) {
        // Wrapped resources share their uniqueID with the wrapping RenderTargetProxy
        REPORTER_ASSERT(reporter, rtProxy->uniqueID().asUInt() == rt->uniqueID().asUInt());
    } else {
        // Deferred resources should always have a different ID from their instantiated rendertarget
        REPORTER_ASSERT(reporter, rtProxy->uniqueID().asUInt() != rt->uniqueID().asUInt());
    }

    if (SkBackingFit::kExact == fit) {
        REPORTER_ASSERT(reporter, rt->width() == rtProxy->width());
        REPORTER_ASSERT(reporter, rt->height() == rtProxy->height());
    } else {
        REPORTER_ASSERT(reporter, rt->width() >= rtProxy->width());
        REPORTER_ASSERT(reporter, rt->height() >= rtProxy->height());
    }
    REPORTER_ASSERT(reporter, rt->config() == rtProxy->config());

    REPORTER_ASSERT(reporter, rt->fsaaType() == rtProxy->fsaaType());
    REPORTER_ASSERT(reporter, rt->numColorSamples() == rtProxy->numColorSamples());
    REPORTER_ASSERT(reporter, rt->numStencilSamples() == rtProxy->numStencilSamples());
    REPORTER_ASSERT(reporter, rt->renderTargetPriv().flags() == rtProxy->testingOnly_getFlags());
}

static void check_texture(skiatest::Reporter* reporter,
                          GrResourceProvider* provider,
                          GrTextureProxy* texProxy,
                          SkBackingFit fit,
                          bool wasWrapped) {
    GrSurfaceProxy::UniqueID idBefore = texProxy->uniqueID();

    REPORTER_ASSERT(reporter, texProxy->instantiate(provider));
    GrTexture* tex = texProxy->priv().peekTexture();

    REPORTER_ASSERT(reporter, texProxy->uniqueID() == idBefore);
    if (wasWrapped) {
        // Wrapped resources share their uniqueID with the wrapping TextureProxy
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() == tex->uniqueID().asUInt());
    } else {
        // Deferred resources should always have a different ID from their instantiated texture
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() != tex->uniqueID().asUInt());
    }

    if (SkBackingFit::kExact == fit) {
        REPORTER_ASSERT(reporter, tex->width() == texProxy->width());
        REPORTER_ASSERT(reporter, tex->height() == texProxy->height());
    } else {
        REPORTER_ASSERT(reporter, tex->width() >= texProxy->width());
        REPORTER_ASSERT(reporter, tex->height() >= texProxy->height());
    }
    REPORTER_ASSERT(reporter, tex->config() == texProxy->config());
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DeferredProxyTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->contextPriv().resourceProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    int attempt = 0; // useful for debugging

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto widthHeight : { 100, 128, 1048576 }) {
            for (auto config : { kAlpha_8_GrPixelConfig, kRGB_565_GrPixelConfig,
                                 kRGBA_8888_GrPixelConfig }) {
                for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
                    for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                        for (auto numSamples : {1, 4, 16, 128}) {
                            GrSurfaceDesc desc;
                            desc.fFlags = kRenderTarget_GrSurfaceFlag;
                            desc.fOrigin = origin;
                            desc.fWidth = widthHeight;
                            desc.fHeight = widthHeight;
                            desc.fConfig = config;
                            desc.fSampleCnt = numSamples;

                            {
                                sk_sp<GrTexture> tex;
                                if (SkBackingFit::kApprox == fit) {
                                    tex = resourceProvider->createApproxTexture(desc, 0);
                                } else {
                                    tex = resourceProvider->createTexture(desc, budgeted);
                                }

                                sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                                                                            desc, fit, budgeted);
                                REPORTER_ASSERT(reporter, SkToBool(tex) == SkToBool(proxy));
                                if (proxy) {
                                    REPORTER_ASSERT(reporter, proxy->asRenderTargetProxy());
                                    // This forces the proxy to compute and cache its
                                    // pre-instantiation size guess. Later, when it is actually
                                    // instantiated, it checks that the instantiated size is <= to
                                    // the pre-computation. If the proxy never computed its
                                    // pre-instantiation size then the check is skipped.
                                    proxy->gpuMemorySize();

                                    check_surface(reporter, proxy.get(), origin,
                                                  widthHeight, widthHeight, config, budgeted);
                                    int supportedSamples = caps.getSampleCount(numSamples, config);
                                    check_rendertarget(reporter, caps, resourceProvider,
                                                       proxy->asRenderTargetProxy(),
                                                       supportedSamples,
                                                       fit, caps.maxWindowRectangles(), false);
                                }
                            }

                            desc.fFlags = kNone_GrSurfaceFlags;

                            {
                                sk_sp<GrTexture> tex;
                                if (SkBackingFit::kApprox == fit) {
                                    tex = resourceProvider->createApproxTexture(desc, 0);
                                } else {
                                    tex = resourceProvider->createTexture(desc, budgeted);
                                }

                                sk_sp<GrTextureProxy> proxy(proxyProvider->createProxy(
                                                                            desc, fit, budgeted));
                                REPORTER_ASSERT(reporter, SkToBool(tex) == SkToBool(proxy));
                                if (proxy) {
                                    // This forces the proxy to compute and cache its
                                    // pre-instantiation size guess. Later, when it is actually
                                    // instantiated, it checks that the instantiated size is <= to
                                    // the pre-computation. If the proxy never computed its
                                    // pre-instantiation size then the check is skipped.
                                    proxy->gpuMemorySize();

                                    check_surface(reporter, proxy.get(), origin,
                                                  widthHeight, widthHeight, config, budgeted);
                                    check_texture(reporter, resourceProvider,
                                                  proxy->asTextureProxy(), fit, false);
                                }
                            }

                            attempt++;
                        }
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedProxyTest, reporter, ctxInfo) {
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->contextPriv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->contextPriv().resourceProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    static const int kWidthHeight = 100;

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto config : { kAlpha_8_GrPixelConfig, kRGBA_8888_GrPixelConfig }) {
            for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                for (auto numSamples : {1, 4}) {
                    int supportedNumSamples = caps.getSampleCount(numSamples, config);

                    bool renderable = caps.isConfigRenderable(config, numSamples > 1);

                    GrSurfaceDesc desc;
                    desc.fOrigin = origin;
                    desc.fWidth = kWidthHeight;
                    desc.fHeight = kWidthHeight;
                    desc.fConfig = config;
                    desc.fSampleCnt = supportedNumSamples;

                    // External on-screen render target.
                    if (renderable && kOpenGL_GrBackend == ctxInfo.backend()) {
                        GrGLFramebufferInfo fboInfo;
                        fboInfo.fFBOID = 0;
                        GrBackendRenderTarget backendRT(kWidthHeight, kWidthHeight, numSamples, 8,
                                                        config, fboInfo);

                        sk_sp<GrSurfaceProxy> sProxy(proxyProvider->createWrappedRenderTargetProxy(
                                                                                backendRT, origin));
                        check_surface(reporter, sProxy.get(), origin,
                                      kWidthHeight, kWidthHeight, config, SkBudgeted::kNo);
                        check_rendertarget(reporter, caps, resourceProvider,
                                           sProxy->asRenderTargetProxy(),
                                           supportedNumSamples, SkBackingFit::kExact, 0, true);
                    }

                    if (renderable) {
                        // Internal offscreen render target.
                        desc.fFlags = kRenderTarget_GrSurfaceFlag;

                        sk_sp<GrSurfaceProxy> sProxy = proxyProvider->createInstantiatedProxy(
                                                        desc, SkBackingFit::kExact, budgeted);
                        if (!sProxy) {
                            continue;  // This can fail on Mesa
                        }

                        check_surface(reporter, sProxy.get(), origin,
                                      kWidthHeight, kWidthHeight, config, budgeted);
                        check_rendertarget(reporter, caps, resourceProvider,
                                           sProxy->asRenderTargetProxy(),
                                           supportedNumSamples, SkBackingFit::kExact,
                                           caps.maxWindowRectangles(), true);
                    } else {
                        // Internal offscreen texture
                        SkASSERT(kNone_GrSurfaceFlags == desc.fFlags );
                        desc.fSampleCnt = 1;

                        sk_sp<GrSurfaceProxy> sProxy = proxyProvider->createInstantiatedProxy(
                                                          desc, SkBackingFit::kExact, budgeted);
                        if (!sProxy) {
                            continue;
                        }

                        check_surface(reporter, sProxy.get(), origin,
                                      kWidthHeight, kWidthHeight, config, budgeted);
                        check_texture(reporter, resourceProvider, sProxy->asTextureProxy(),
                                      SkBackingFit::kExact, true);
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ZeroSizedProxyTest, reporter, ctxInfo) {
    GrProxyProvider* provider = ctxInfo.grContext()->contextPriv().proxyProvider();

    for (auto flags : { kRenderTarget_GrSurfaceFlag, kNone_GrSurfaceFlags }) {
        for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
            for (int width : { 0, 100 }) {
                for (int height : { 0, 100}) {
                    if (width && height) {
                        continue; // not zero-sized
                    }

                    GrSurfaceDesc desc;
                    desc.fFlags = flags;
                    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
                    desc.fWidth = width;
                    desc.fHeight = height;
                    desc.fConfig = kRGBA_8888_GrPixelConfig;
                    desc.fSampleCnt = 1;

                    sk_sp<GrTextureProxy> proxy = provider->createProxy(desc, fit, SkBudgeted::kNo);
                    REPORTER_ASSERT(reporter, !proxy);
                }
            }
        }
    }
}

#endif
