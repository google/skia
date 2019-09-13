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
#include "src/gpu/gl/GrGLUtil.h"

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
#ifdef SK_DEBUG
    REPORTER_ASSERT(reporter, GrCaps::AreConfigsCompatible(config, proxy->config()));
#endif
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
    // The instantiated texture should have these dimensions. If the fit is kExact, then
    // 'worst-case' reports the original WxH. If it is kApprox, make sure that the texture
    // is that size and didn't reuse one of the kExact surfaces in the provider. This is important
    // because upstream usage (e.g. SkImage) reports size based on the worst case dimensions and
    // client code may rely on that if they are creating backend resources.
    // NOTE: we store these before instantiating, since after instantiation worstCaseWH() just
    // return the target's dimensions. In this instance, we want to ensure the target's dimensions
    // are no different from the original approximate (or exact) dimensions.
    int expectedWidth = texProxy->worstCaseWidth();
    int expectedHeight = texProxy->worstCaseHeight();

    REPORTER_ASSERT(reporter, texProxy->instantiate(provider));
    GrTexture* tex = texProxy->peekTexture();

    REPORTER_ASSERT(reporter, texProxy->uniqueID() == idBefore);
    // Deferred resources should always have a different ID from their instantiated texture
    if (preinstantiated) {
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() == tex->uniqueID().asUInt());
    } else {
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() != tex->uniqueID().asUInt());
    }

    REPORTER_ASSERT(reporter, tex->width() == expectedWidth);
    REPORTER_ASSERT(reporter, tex->height() == expectedHeight);

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

                            auto format = caps.getDefaultBackendFormat(ct, GrRenderable::kYes);
                            if (!format.isValid()) {
                                continue;
                            }

                            // Renderable
                            {
                                sk_sp<GrTexture> tex;
                                if (SkBackingFit::kApprox == fit) {
                                    tex = resourceProvider->createApproxTexture(
                                            desc, format, GrRenderable::kYes, numSamples,
                                            GrProtected::kNo);
                                } else {
                                    tex = resourceProvider->createTexture(
                                            desc, format, GrRenderable::kYes, numSamples, budgeted,
                                            GrProtected::kNo);
                                }

                                sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                                        format, desc, GrRenderable::kYes, numSamples, origin,
                                        GrMipMapped::kNo, fit, budgeted, GrProtected::kNo);
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
                                            caps.getRenderTargetSampleCount(numSamples, format);
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
                                            desc, format, GrRenderable::kNo, numSamples,
                                            GrProtected::kNo);
                                } else {
                                    tex = resourceProvider->createTexture(
                                            desc, format, GrRenderable::kNo, numSamples, budgeted,
                                            GrProtected::kNo);
                                }

                                sk_sp<GrTextureProxy> proxy(proxyProvider->createProxy(
                                        format, desc, GrRenderable::kNo, numSamples, origin,
                                        GrMipMapped::kNo, fit, budgeted, GrProtected::kNo));
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
            GrPixelConfig config = GrColorTypeToPixelConfig(grColorType);
            SkASSERT(kUnknown_GrPixelConfig != config);

            // External on-screen render target.
            // Tests wrapBackendRenderTarget with a GrBackendRenderTarget
            // Our test-only function that creates a backend render target doesn't currently support
            // sample counts :(.
            if (ctxInfo.grContext()->colorTypeSupportedAsSurface(colorType)) {
                GrBackendRenderTarget backendRT = gpu->createTestingOnlyBackendRenderTarget(
                        kWidthHeight, kWidthHeight, grColorType);
                sk_sp<GrSurfaceProxy> sProxy(
                        proxyProvider->wrapBackendRenderTarget(backendRT, grColorType,
                                                               origin, nullptr, nullptr));
                check_surface(reporter, sProxy.get(), origin, kWidthHeight, kWidthHeight,
                              config, SkBudgeted::kNo);
                static constexpr int kExpectedNumSamples = 1;
                check_rendertarget(reporter, caps, resourceProvider, sProxy->asRenderTargetProxy(),
                                   kExpectedNumSamples, SkBackingFit::kExact,
                                   caps.maxWindowRectangles());
                gpu->deleteTestingOnlyBackendRenderTarget(backendRT);
            }

            for (auto numSamples : {1, 4}) {
                auto beFormat = caps.getDefaultBackendFormat(grColorType, GrRenderable::kYes);
                int supportedNumSamples = caps.getRenderTargetSampleCount(numSamples, beFormat);
                if (!supportedNumSamples) {
                    continue;
                }

                // Test wrapping FBO 0 (with made up properties). This tests sample count and the
                // special case where FBO 0 doesn't support window rectangles.
                if (GrBackendApi::kOpenGL == ctxInfo.backend()) {
                    GrGLFramebufferInfo fboInfo;
                    fboInfo.fFBOID = 0;
                    fboInfo.fFormat = GrGLFormatToEnum(beFormat.asGLFormat());
                    SkASSERT(fboInfo.fFormat);
                    static constexpr int kStencilBits = 8;
                    GrBackendRenderTarget backendRT(kWidthHeight, kWidthHeight, numSamples,
                                                    kStencilBits, fboInfo);
                    sk_sp<GrSurfaceProxy> sProxy(
                            proxyProvider->wrapBackendRenderTarget(backendRT, grColorType,
                                                                   origin, nullptr, nullptr));
                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  config, SkBudgeted::kNo);
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
                            backendTex, grColorType, origin, supportedNumSamples);
                    if (!sProxy) {
                        context->deleteBackendTexture(backendTex);
                        continue;  // This can fail on Mesa
                    }

                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  config, SkBudgeted::kNo);
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
                                  config, SkBudgeted::kNo);
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
                            backendTex, grColorType, origin, kBorrow_GrWrapOwnership,
                            GrWrapCacheable::kNo, kRead_GrIOType);
                    if (!sProxy) {
                        context->deleteBackendTexture(backendTex);
                        continue;
                    }

                    check_surface(reporter, sProxy.get(), origin,
                                  kWidthHeight, kWidthHeight,
                                  config, SkBudgeted::kNo);
                    check_texture(reporter, resourceProvider, sProxy->asTextureProxy(),
                                  SkBackingFit::kExact);

                    context->deleteBackendTexture(backendTex);
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ZeroSizedProxyTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* provider = context->priv().proxyProvider();

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

                    const GrBackendFormat format =
                            context->priv().caps()->getDefaultBackendFormat(
                                GrColorType::kRGBA_8888,
                                renderable);

                    sk_sp<GrTextureProxy> proxy = provider->createProxy(
                            format, desc, renderable, 1, kBottomLeft_GrSurfaceOrigin,
                            GrMipMapped::kNo, fit, SkBudgeted::kNo, GrProtected::kNo);
                    REPORTER_ASSERT(reporter, !proxy);
                }
            }
        }
    }
}
