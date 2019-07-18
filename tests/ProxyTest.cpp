/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/gl/GrGLDefines.h"

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
                               int expectedMaxWindowRects) {
    REPORTER_ASSERT(reporter, rtProxy->maxWindowRectangles(caps) == expectedMaxWindowRects);
    REPORTER_ASSERT(reporter, rtProxy->numSamples() == numSamples);

    GrSurfaceProxy::UniqueID idBefore = rtProxy->uniqueID();
    bool preinstantiated = rtProxy->isInstantiated();
    REPORTER_ASSERT(reporter, rtProxy->instantiate(provider));
    GrRenderTarget* rt = rtProxy->peekRenderTarget();

    REPORTER_ASSERT(reporter, rtProxy->uniqueID() == idBefore);
    // Deferred resources should always have a different ID from their instantiated rendertarget
    if (preinstantiated) {
        REPORTER_ASSERT(reporter, rtProxy->uniqueID().asUInt() == rt->uniqueID().asUInt());
    } else {
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

    REPORTER_ASSERT(reporter, rt->numSamples() == rtProxy->numSamples());
    REPORTER_ASSERT(reporter, rt->surfacePriv().flags() == rtProxy->testingOnly_getFlags());
}

static void check_texture(skiatest::Reporter* reporter,
                          GrResourceProvider* provider,
                          GrTextureProxy* texProxy,
                          SkBackingFit fit) {
    GrSurfaceProxy::UniqueID idBefore = texProxy->uniqueID();

    bool preinstantiated = texProxy->isInstantiated();
    REPORTER_ASSERT(reporter, texProxy->instantiate(provider));
    GrTexture* tex = texProxy->peekTexture();

    REPORTER_ASSERT(reporter, texProxy->uniqueID() == idBefore);
    // Deferred resources should always have a different ID from their instantiated texture
    if (preinstantiated) {
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() == tex->uniqueID().asUInt());
    } else {
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
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrResourceProvider* resourceProvider = ctxInfo.grContext()->priv().resourceProvider();
    const GrCaps& caps = *ctxInfo.grContext()->priv().caps();

    int attempt = 0; // useful for debugging

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto widthHeight : { 100, 128, 1048576 }) {
            for (auto ct : { GrColorType::kAlpha_8, GrColorType::kBGR_565,
                             GrColorType::kRGBA_8888, GrColorType::kRGBA_1010102 } ) {
                for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
                    for (auto budgeted : { SkBudgeted::kYes, SkBudgeted::kNo }) {
                        for (auto numSamples : {1, 4, 16, 128}) {

                            auto config = GrColorTypeToPixelConfig(ct);
                            SkASSERT(kUnknown_GrPixelConfig != config);

                            GrSurfaceDesc desc;
                            desc.fWidth = widthHeight;
                            desc.fHeight = widthHeight;
                            desc.fConfig = config;
                            desc.fSampleCnt = numSamples;

                            const GrBackendFormat format = caps.getBackendFormatFromColorType(ct);
                            if (!format.isValid()) {
                                continue;
                            }

                            // Renderable
                            {
                                sk_sp<GrTexture> tex;
                                if (SkBackingFit::kApprox == fit) {
                                    tex = resourceProvider->createApproxTexture(
                                            desc, GrRenderable::kYes, GrProtected::kNo,
                                            GrResourceProvider::Flags::kNoPendingIO);
                                } else {
                                    tex = resourceProvider->createTexture(
                                            desc, GrRenderable::kYes, budgeted, GrProtected::kNo,
                                            GrResourceProvider::Flags::kNoPendingIO);
                                }

                                sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                                        format, desc, GrRenderable::kYes, origin, fit, budgeted,
                                        GrProtected::kNo);
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
                                    int supportedSamples =
                                            caps.getRenderTargetSampleCount(numSamples, config);
                                    check_rendertarget(reporter, caps, resourceProvider,
                                                       proxy->asRenderTargetProxy(),
                                                       supportedSamples,
                                                       fit, caps.maxWindowRectangles());
                                }
                            }

                            // Not renderable
                            {
                                sk_sp<GrTexture> tex;
                                if (SkBackingFit::kApprox == fit) {
                                    tex = resourceProvider->createApproxTexture(
                                            desc, GrRenderable::kNo, GrProtected::kNo,
                                            GrResourceProvider::Flags::kNoPendingIO);
                                } else {
                                    tex = resourceProvider->createTexture(
                                            desc, GrRenderable::kNo, budgeted, GrProtected::kNo,
                                            GrResourceProvider::Flags::kNoPendingIO);
                                }

                                sk_sp<GrTextureProxy> proxy(proxyProvider->createProxy(
                                        format, desc, GrRenderable::kNo, origin, fit, budgeted,
                                        GrProtected::kNo));
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
                                                  proxy->asTextureProxy(), fit);
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
    GrProxyProvider* proxyProvider = ctxInfo.grContext()->priv().proxyProvider();
    GrContext* context = ctxInfo.grContext();
    GrResourceProvider* resourceProvider = context->priv().resourceProvider();
    GrGpu* gpu = context->priv().getGpu();
    const GrCaps& caps = *context->priv().caps();

    static const int kWidthHeight = 100;

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        for (auto colorType : { kAlpha_8_SkColorType, kRGBA_8888_SkColorType,
                                kRGBA_1010102_SkColorType }) {
            GrColorType grColorType = SkColorTypeToGrColorType(colorType);
            // External on-screen render target.
            // Tests wrapBackendRenderTarget with a GrBackendRenderTarget
            // Our test-only function that creates a backend render target doesn't currently support
            // sample counts :(.
            if (ctxInfo.grContext()->colorTypeSupportedAsSurface(colorType)) {
                GrBackendRenderTarget backendRT = gpu->createTestingOnlyBackendRenderTarget(
                        kWidthHeight, kWidthHeight, grColorType);
                sk_sp<GrSurfaceProxy> sProxy(
                        proxyProvider->wrapBackendRenderTarget(backendRT, origin, nullptr,
                                                               nullptr));
                check_surface(reporter, sProxy.get(), origin, kWidthHeight, kWidthHeight,
                              backendRT.pixelConfig(), SkBudgeted::kNo);
                static constexpr int kExpectedNumSamples = 1;
                check_rendertarget(reporter, caps, resourceProvider, sProxy->asRenderTargetProxy(),
                                   kExpectedNumSamples, SkBackingFit::kExact,
                                   caps.maxWindowRectangles());
                gpu->deleteTestingOnlyBackendRenderTarget(backendRT);
            }

            for (auto numSamples : {1, 4}) {
                GrPixelConfig config = GrColorTypeToPixelConfig(grColorType);
                SkASSERT(kUnknown_GrPixelConfig != config);
                int supportedNumSamples = caps.getRenderTargetSampleCount(numSamples, config);

                if (!supportedNumSamples) {
                    continue;
                }

                // Test wrapping FBO 0 (with made up properties). This tests sample count and the
                // special case where FBO 0 doesn't support window rectangles.
                if (GrBackendApi::kOpenGL == ctxInfo.backend()) {
                    GrBackendFormat beFormat = caps.getBackendFormatFromColorType(grColorType);
                    GrGLFramebufferInfo fboInfo;
                    fboInfo.fFBOID = 0;
                    SkASSERT(beFormat.getGLFormat());
                    fboInfo.fFormat = *beFormat.getGLFormat();
                    static constexpr int kStencilBits = 8;
                    GrBackendRenderTarget backendRT(kWidthHeight, kWidthHeight, numSamples,
                                                    kStencilBits, fboInfo);
                    backendRT.setPixelConfig(config);
                    sk_sp<GrSurfaceProxy> sProxy(
                            proxyProvider->wrapBackendRenderTarget(backendRT, origin, nullptr,
                                                                   nullptr));
                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  backendRT.pixelConfig(), SkBudgeted::kNo);
                    check_rendertarget(reporter, caps, resourceProvider,
                                       sProxy->asRenderTargetProxy(),
                                       supportedNumSamples, SkBackingFit::kExact, 0);
                }

                // Tests wrapBackendRenderTarget with a GrBackendTexture
                {
                    GrBackendTexture backendTex =
                            context->createBackendTexture(kWidthHeight, kWidthHeight,
                                                          colorType,
                                                          SkColors::kTransparent,
                                                          GrMipMapped::kNo,
                                                          GrRenderable::kYes,
                                                          GrProtected::kNo);
                    sk_sp<GrSurfaceProxy> sProxy = proxyProvider->wrapBackendTextureAsRenderTarget(
                            backendTex, origin, supportedNumSamples);
                    if (!sProxy) {
                        context->deleteBackendTexture(backendTex);
                        continue;  // This can fail on Mesa
                    }

                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  backendTex.pixelConfig(), SkBudgeted::kNo);
                    check_rendertarget(reporter, caps, resourceProvider,
                                       sProxy->asRenderTargetProxy(),
                                       supportedNumSamples, SkBackingFit::kExact,
                                       caps.maxWindowRectangles());

                    context->deleteBackendTexture(backendTex);
                }

                // Tests wrapBackendTexture that is only renderable
                {
                    GrBackendTexture backendTex =
                            context->createBackendTexture(kWidthHeight, kWidthHeight,
                                                          colorType,
                                                          SkColors::kTransparent,
                                                          GrMipMapped::kNo,
                                                          GrRenderable::kYes,
                                                          GrProtected::kNo);

                    sk_sp<GrSurfaceProxy> sProxy = proxyProvider->wrapRenderableBackendTexture(
                            backendTex, origin, supportedNumSamples,
                            grColorType, kBorrow_GrWrapOwnership,
                            GrWrapCacheable::kNo, nullptr, nullptr);
                    if (!sProxy) {
                        context->deleteBackendTexture(backendTex);
                        continue;  // This can fail on Mesa
                    }

                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  backendTex.pixelConfig(), SkBudgeted::kNo);
                    check_rendertarget(reporter, caps, resourceProvider,
                                       sProxy->asRenderTargetProxy(),
                                       supportedNumSamples, SkBackingFit::kExact,
                                       caps.maxWindowRectangles());

                    context->deleteBackendTexture(backendTex);
                }

                // Tests wrapBackendTexture that is only textureable
                {
                    // Internal offscreen texture
                    GrBackendTexture backendTex =
                            context->createBackendTexture(kWidthHeight, kWidthHeight,
                                                          colorType,
                                                          SkColors::kTransparent,
                                                          GrMipMapped::kNo,
                                                          GrRenderable::kNo,
                                                          GrProtected::kNo);

                    sk_sp<GrSurfaceProxy> sProxy = proxyProvider->wrapBackendTexture(
                            backendTex, origin, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                            kRead_GrIOType);
                    if (!sProxy) {
                        context->deleteBackendTexture(backendTex);
                        continue;
                    }

                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  backendTex.pixelConfig(), SkBudgeted::kNo);
                    check_texture(reporter, resourceProvider, sProxy->asTextureProxy(),
                                  SkBackingFit::kExact);

                    context->deleteBackendTexture(backendTex);
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ZeroSizedProxyTest, reporter, ctxInfo) {
    GrProxyProvider* provider = ctxInfo.grContext()->priv().proxyProvider();

    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
            for (int width : { 0, 100 }) {
                for (int height : { 0, 100}) {
                    if (width && height) {
                        continue; // not zero-sized
                    }

                    GrSurfaceDesc desc;
                    desc.fWidth = width;
                    desc.fHeight = height;
                    desc.fConfig = kRGBA_8888_GrPixelConfig;
                    desc.fSampleCnt = 1;

                    const GrBackendFormat format =
                        ctxInfo.grContext()->priv().caps()->getBackendFormatFromColorType(
                                GrColorType::kRGBA_8888);

                    sk_sp<GrTextureProxy> proxy = provider->createProxy(
                            format, desc, renderable, kBottomLeft_GrSurfaceOrigin, fit,
                            SkBudgeted::kNo, GrProtected::kNo);
                    REPORTER_ASSERT(reporter, !proxy);
                }
            }
        }
    }
}
