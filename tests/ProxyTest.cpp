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

static void check_surface(skiatest::Reporter* reporter,
                          GrSurfaceProxy* proxy,
                          GrSurfaceOrigin origin,
                          int width, int height, 
                          GrPixelConfig config) {
    REPORTER_ASSERT(reporter, proxy->origin() == origin);
    REPORTER_ASSERT(reporter, proxy->width() == width);
    REPORTER_ASSERT(reporter, proxy->height() == height);
    REPORTER_ASSERT(reporter, proxy->config() == config);
}

static void check_rendertarget(skiatest::Reporter* reporter,
                               GrTextureProvider* provider,
                               GrRenderTargetProxy* rtProxy,
                               SkBackingFit fit) {
    REPORTER_ASSERT(reporter, rtProxy->asTextureProxy() == nullptr); // for now
    REPORTER_ASSERT(reporter, rtProxy->asRenderTargetProxy() == rtProxy);

    GrRenderTarget* rt = rtProxy->instantiate(provider);
    REPORTER_ASSERT(reporter, rt);

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
    REPORTER_ASSERT(reporter, rt->hasMixedSamples() == rtProxy->hasMixedSamples());
}

static void check_texture(skiatest::Reporter* reporter,
                          GrTextureProvider* provider,
                          GrTextureProxy* texProxy,
                          SkBackingFit fit) {
    REPORTER_ASSERT(reporter, texProxy->asTextureProxy() == texProxy);
    REPORTER_ASSERT(reporter, texProxy->asRenderTargetProxy() == nullptr); // for now

    GrTexture* tex = texProxy->instantiate(provider);
    REPORTER_ASSERT(reporter, tex);

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


DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(AllocedProxyTest, reporter, ctxInfo) {
    GrTextureProvider* provider = ctxInfo.fGrContext->textureProvider();

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto widthHeight : { 100, 128 }) {
            for (auto config : { kAlpha_8_GrPixelConfig, kRGBA_8888_GrPixelConfig }) {
                for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
                    for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                        for (auto numSamples : { 0, 4}) {
                            bool renderable = ctxInfo.fGrContext->caps()->isConfigRenderable(
                                                                      config, numSamples > 0);

                            GrSurfaceDesc desc;
                            desc.fOrigin = origin;
                            desc.fWidth = widthHeight;
                            desc.fHeight = widthHeight;
                            desc.fConfig = config;
                            desc.fSampleCnt = numSamples;

                            if (renderable) {
                                sk_sp<GrRenderTargetProxy> rtProxy(GrRenderTargetProxy::Make(
                                                                    *ctxInfo.fGrContext->caps(),
                                                                    desc, 
                                                                    fit,
                                                                    budgeted));
                                check_surface(reporter, rtProxy.get(), origin,
                                              widthHeight, widthHeight, config);
                                check_rendertarget(reporter, provider, rtProxy.get(), fit);
                            }

                            desc.fSampleCnt = 0;

                            sk_sp<GrTextureProxy> texProxy(GrTextureProxy::Make(desc,
                                                                                fit,
                                                                                budgeted));
                            check_surface(reporter, texProxy.get(), origin,
                                          widthHeight, widthHeight, config);
                            check_texture(reporter, provider, texProxy.get(), fit);
                        }
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(WrappedProxyTest, reporter, ctxInfo) {
    GrTextureProvider* provider = ctxInfo.fGrContext->textureProvider();

    static const int kWidthHeight = 100;

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto config : { kAlpha_8_GrPixelConfig, kRGBA_8888_GrPixelConfig }) {
            for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                for (auto numSamples: { 0, 4}) {
                    bool renderable = ctxInfo.fGrContext->caps()->isConfigRenderable(
                                                                config, numSamples > 0);

                    GrSurfaceDesc desc;
                    desc.fOrigin = origin;
                    desc.fWidth = kWidthHeight;
                    desc.fHeight = kWidthHeight;
                    desc.fConfig = config;
                    desc.fSampleCnt = numSamples;

                    sk_sp<GrTexture> tex;

                    if (renderable) {
                        desc.fFlags = kRenderTarget_GrSurfaceFlag;
                        tex.reset(provider->createTexture(desc, budgeted));
                        sk_sp<GrRenderTarget> rt(sk_ref_sp(tex->asRenderTarget()));

                        sk_sp<GrRenderTargetProxy> rtProxy(GrRenderTargetProxy::Make(rt));
                        check_surface(reporter, rtProxy.get(), origin,
                                      kWidthHeight, kWidthHeight, config);
                        check_rendertarget(reporter, provider, rtProxy.get(), SkBackingFit::kExact);
                    }

                    if (!tex) {
                        SkASSERT(kNone_GrSurfaceFlags == desc.fFlags );
                        desc.fSampleCnt = 0;
                        tex.reset(provider->createTexture(desc, budgeted));
                    }

                    sk_sp<GrTextureProxy> texProxy(GrTextureProxy::Make(tex));
                    check_surface(reporter, texProxy.get(), origin,
                                  kWidthHeight, kWidthHeight, config);
                    check_texture(reporter, provider, texProxy.get(), SkBackingFit::kExact);
                }
            }
        }
    }
}

#endif
