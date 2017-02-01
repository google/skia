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

// Check that the surface proxy's member vars are set as expected
static void check_surface(skiatest::Reporter* reporter,
                          GrSurfaceProxy* proxy,
                          GrSurfaceOrigin origin,
                          int width, int height, 
                          GrPixelConfig config,
                          const GrGpuResource::UniqueID& uniqueID,
                          SkBudgeted budgeted) {
    REPORTER_ASSERT(reporter, proxy->origin() == origin);
    REPORTER_ASSERT(reporter, proxy->width() == width);
    REPORTER_ASSERT(reporter, proxy->height() == height);
    REPORTER_ASSERT(reporter, proxy->config() == config);
    if (!uniqueID.isInvalid()) {
        REPORTER_ASSERT(reporter, proxy->uniqueID().asUInt() == uniqueID.asUInt());
    } else {
        REPORTER_ASSERT(reporter, !proxy->uniqueID().isInvalid());
    }
    REPORTER_ASSERT(reporter, proxy->isBudgeted() == budgeted);
}

static void check_rendertarget(skiatest::Reporter* reporter,
                               const GrCaps& caps,
                               GrTextureProvider* provider,
                               GrRenderTargetProxy* rtProxy,
                               int numSamples,
                               SkBackingFit fit,
                               int expectedMaxWindowRects,
                               bool wasWrapped) {
    REPORTER_ASSERT(reporter, rtProxy->maxWindowRectangles(caps) == expectedMaxWindowRects);
    REPORTER_ASSERT(reporter, rtProxy->numStencilSamples() == numSamples);

    GrSurfaceProxy::UniqueID idBefore = rtProxy->uniqueID();
    GrRenderTarget* rt = rtProxy->instantiate(provider);
    REPORTER_ASSERT(reporter, rt);

    REPORTER_ASSERT(reporter, rtProxy->uniqueID() == idBefore);
    if (wasWrapped) {
        // Wrapped resources share their uniqueID with the wrapping RenderTargetProxy
        REPORTER_ASSERT(reporter, rtProxy->uniqueID().asUInt() == rt->uniqueID().asUInt());
    } else {
        // Deferred resources should always have a different ID from their instantiated rendertarget
        REPORTER_ASSERT(reporter, rtProxy->uniqueID().asUInt() != rt->uniqueID().asUInt());
    }

    REPORTER_ASSERT(reporter, rt->origin() == rtProxy->origin());
    if (SkBackingFit::kExact == fit) {
        REPORTER_ASSERT(reporter, rt->width() == rtProxy->width());
        REPORTER_ASSERT(reporter, rt->height() == rtProxy->height());
    } else {
        REPORTER_ASSERT(reporter, rt->width() >= rtProxy->width());
        REPORTER_ASSERT(reporter, rt->height() >= rtProxy->height());
    }
    REPORTER_ASSERT(reporter, rt->config() == rtProxy->config());

    REPORTER_ASSERT(reporter, rt->isUnifiedMultisampled() == rtProxy->isUnifiedMultisampled());
    REPORTER_ASSERT(reporter, rt->isStencilBufferMultisampled() ==
                              rtProxy->isStencilBufferMultisampled());
    REPORTER_ASSERT(reporter, rt->numColorSamples() == rtProxy->numColorSamples());
    REPORTER_ASSERT(reporter, rt->numStencilSamples() == rtProxy->numStencilSamples());
    REPORTER_ASSERT(reporter, rt->isMixedSampled() == rtProxy->isMixedSampled());
    REPORTER_ASSERT(reporter, rt->renderTargetPriv().flags() == rtProxy->testingOnly_getFlags());
}

static void check_texture(skiatest::Reporter* reporter,
                          GrTextureProvider* provider,
                          GrTextureProxy* texProxy,
                          SkBackingFit fit,
                          bool wasWrapped) {
    GrSurfaceProxy::UniqueID idBefore = texProxy->uniqueID();
    GrTexture* tex = texProxy->instantiate(provider);
    REPORTER_ASSERT(reporter, tex);

    REPORTER_ASSERT(reporter, texProxy->uniqueID() == idBefore);
    if (wasWrapped) {
        // Wrapped resources share their uniqueID with the wrapping TextureProxy
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() == tex->uniqueID().asUInt());
    } else {
        // Deferred resources should always have a different ID from their instantiated texture
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() != tex->uniqueID().asUInt());
    }

    REPORTER_ASSERT(reporter, tex->origin() == texProxy->origin());
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
    GrTextureProvider* provider = ctxInfo.grContext()->textureProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    const GrGpuResource::UniqueID kInvalidResourceID = GrGpuResource::UniqueID::InvalidID();

    int attempt = 0; // useful for debugging

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto widthHeight : { 100, 128, 1048576 }) {
            for (auto config : { kAlpha_8_GrPixelConfig, kRGB_565_GrPixelConfig,
                                 kETC1_GrPixelConfig, kRGBA_8888_GrPixelConfig }) {
                for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
                    for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                        for (auto numSamples : { 0, 4, 16, 128 }) {
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
                                    tex.reset(provider->createApproxTexture(desc));
                                } else {
                                    tex.reset(provider->createTexture(desc, budgeted));
                                }

                                sk_sp<GrSurfaceProxy> sProxy(GrSurfaceProxy::MakeDeferred(
                                                                                caps, desc,
                                                                                fit, budgeted));
                                REPORTER_ASSERT(reporter, SkToBool(tex) == SkToBool(sProxy));
                                if (sProxy) {
                                    REPORTER_ASSERT(reporter, sProxy->asRenderTargetProxy());
                                    // This forces the proxy to compute and cache its
                                    // pre-instantiation size guess. Later, when it is actually
                                    // instantiated, it checks that the instantiated size is <= to
                                    // the pre-computation. If the proxy never computed its
                                    // pre-instantiation size then the check is skipped.
                                    sProxy->gpuMemorySize();

                                    check_surface(reporter, sProxy.get(), origin,
                                                  widthHeight, widthHeight, config,
                                                  kInvalidResourceID, budgeted);
                                    check_rendertarget(reporter, caps, provider,
                                                       sProxy->asRenderTargetProxy(),
                                                       SkTMin(numSamples, caps.maxSampleCount()),
                                                       fit, caps.maxWindowRectangles(), false);
                                }
                            }

                            desc.fFlags = kNone_GrSurfaceFlags;

                            {
                                sk_sp<GrTexture> tex;
                                if (SkBackingFit::kApprox == fit) {
                                    tex.reset(provider->createApproxTexture(desc));
                                } else {
                                    tex.reset(provider->createTexture(desc, budgeted));
                                }

                                sk_sp<GrSurfaceProxy> sProxy(GrSurfaceProxy::MakeDeferred(caps,
                                                                                          desc,
                                                                                          fit,
                                                                                          budgeted));
                                REPORTER_ASSERT(reporter, SkToBool(tex) == SkToBool(sProxy));
                                if (sProxy) {
                                    // This forces the proxy to compute and cache its pre-instantiation
                                    // size guess. Later, when it is actually instantiated, it checks
                                    // that the instantiated size is <= to the pre-computation.
                                    // If the proxy never computed its pre-instantiation size then the
                                    // check is skipped.
                                    sProxy->gpuMemorySize();

                                    check_surface(reporter, sProxy.get(), origin,
                                                  widthHeight, widthHeight, config,
                                                  kInvalidResourceID, budgeted);
                                    check_texture(reporter, provider, sProxy->asTextureProxy(),
                                                  fit, false);
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
    GrTextureProvider* provider = ctxInfo.grContext()->textureProvider();
    const GrCaps& caps = *ctxInfo.grContext()->caps();

    static const int kWidthHeight = 100;

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto config : { kAlpha_8_GrPixelConfig, kRGBA_8888_GrPixelConfig }) {
            for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                for (auto numSamples: { 0, 4}) {
                    bool renderable = caps.isConfigRenderable(config, numSamples > 0);

                    GrSurfaceDesc desc;
                    desc.fOrigin = origin;
                    desc.fWidth = kWidthHeight;
                    desc.fHeight = kWidthHeight;
                    desc.fConfig = config;
                    desc.fSampleCnt = numSamples;

                    // External on-screen render target.
                    if (renderable && kOpenGL_GrBackend == ctxInfo.backend()) {
                        GrBackendRenderTargetDesc backendDesc;
                        backendDesc.fWidth = kWidthHeight;
                        backendDesc.fHeight = kWidthHeight;
                        backendDesc.fConfig = config;
                        backendDesc.fOrigin = origin;
                        backendDesc.fSampleCnt = numSamples;
                        backendDesc.fStencilBits = 8;
                        backendDesc.fRenderTargetHandle = 0;

                        sk_sp<GrRenderTarget> defaultFBO(
                            provider->wrapBackendRenderTarget(backendDesc));

                        sk_sp<GrSurfaceProxy> sProxy(GrSurfaceProxy::MakeWrapped(defaultFBO));
                        check_surface(reporter, sProxy.get(), origin,
                                      kWidthHeight, kWidthHeight, config,
                                      defaultFBO->uniqueID(), SkBudgeted::kNo);
                        check_rendertarget(reporter, caps, provider, sProxy->asRenderTargetProxy(),
                                           numSamples, SkBackingFit::kExact, 0, true);
                    }

                    sk_sp<GrTexture> tex;

                    // Internal offscreen render target.
                    if (renderable) {
                        desc.fFlags = kRenderTarget_GrSurfaceFlag;
                        tex.reset(provider->createTexture(desc, budgeted));
                        sk_sp<GrRenderTarget> rt(sk_ref_sp(tex->asRenderTarget()));

                        sk_sp<GrSurfaceProxy> sProxy(GrSurfaceProxy::MakeWrapped(rt));
                        check_surface(reporter, sProxy.get(), origin,
                                      kWidthHeight, kWidthHeight, config,
                                      rt->uniqueID(), budgeted);
                        check_rendertarget(reporter, caps, provider, sProxy->asRenderTargetProxy(),
                                           numSamples, SkBackingFit::kExact,
                                           caps.maxWindowRectangles(), true);
                    }

                    if (!tex) {
                        SkASSERT(kNone_GrSurfaceFlags == desc.fFlags );
                        desc.fSampleCnt = 0;
                        tex.reset(provider->createTexture(desc, budgeted));
                    }

                    sk_sp<GrSurfaceProxy> sProxy(GrSurfaceProxy::MakeWrapped(tex));
                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight, config, tex->uniqueID(), budgeted);
                    check_texture(reporter, provider, sProxy->asTextureProxy(),
                                  SkBackingFit::kExact, true);
                }
            }
        }
    }
}

#endif
