/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurface.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tools/gpu/ManagedBackendTexture.h"
#ifdef SK_GL
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"
#endif

#include "tests/TestUtils.h"

// Check that the surface proxy's member vars are set as expected
static void check_surface(skiatest::Reporter* reporter,
                          GrSurfaceProxy* proxy,
                          int width, int height,
                          SkBudgeted budgeted) {
    REPORTER_ASSERT(reporter, proxy->width() == width);
    REPORTER_ASSERT(reporter, proxy->height() == height);
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
        REPORTER_ASSERT(reporter, rt->dimensions() == rtProxy->dimensions());
    } else {
        REPORTER_ASSERT(reporter, rt->width() >= rtProxy->width());
        REPORTER_ASSERT(reporter, rt->height() >= rtProxy->height());
    }
    REPORTER_ASSERT(reporter, rt->backendFormat() == rtProxy->backendFormat());

    REPORTER_ASSERT(reporter, rt->numSamples() == rtProxy->numSamples());
    REPORTER_ASSERT(reporter, rt->flags() == rtProxy->testingOnly_getFlags());
}

static void check_texture(skiatest::Reporter* reporter,
                          GrResourceProvider* provider,
                          GrTextureProxy* texProxy,
                          SkBackingFit fit) {
    GrSurfaceProxy::UniqueID idBefore = texProxy->uniqueID();

    bool preinstantiated = texProxy->isInstantiated();
    // The instantiated texture should have these dimensions. If the fit is kExact, then
    // 'backingStoreDimensions' reports the original WxH. If it is kApprox, make sure that
    // the texture is that size and didn't reuse one of the kExact surfaces in the provider.
    // This is important because upstream usage (e.g. SkImage) reports size based on the
    // backingStoreDimensions and client code may rely on that if they are creating backend
    // resources.
    // NOTE: we store these before instantiating, since after instantiation backingStoreDimensions
    // just returns the target's dimensions. In this instance, we want to ensure the target's
    // dimensions are no different from the original approximate (or exact) dimensions.
    SkISize expectedSize = texProxy->backingStoreDimensions();

    REPORTER_ASSERT(reporter, texProxy->instantiate(provider));
    GrTexture* tex = texProxy->peekTexture();

    REPORTER_ASSERT(reporter, texProxy->uniqueID() == idBefore);
    // Deferred resources should always have a different ID from their instantiated texture
    if (preinstantiated) {
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() == tex->uniqueID().asUInt());
    } else {
        REPORTER_ASSERT(reporter, texProxy->uniqueID().asUInt() != tex->uniqueID().asUInt());
    }

    REPORTER_ASSERT(reporter, tex->dimensions() == expectedSize);

    REPORTER_ASSERT(reporter, tex->backendFormat() == texProxy->backendFormat());
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DeferredProxyTest, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
    const GrCaps& caps = *direct->priv().caps();

    int attempt = 0; // useful for debugging

    for (auto widthHeight : {100, 128, 1048576}) {
        for (auto ct : {GrColorType::kAlpha_8, GrColorType::kBGR_565, GrColorType::kRGBA_8888,
                        GrColorType::kRGBA_1010102}) {
            for (auto fit : {SkBackingFit::kExact, SkBackingFit::kApprox}) {
                for (auto budgeted : {SkBudgeted::kYes, SkBudgeted::kNo}) {
                    for (auto numSamples : {1, 4, 16, 128}) {
                        SkISize dims = {widthHeight, widthHeight};

                        auto format = caps.getDefaultBackendFormat(ct, GrRenderable::kYes);
                        if (!format.isValid()) {
                            continue;
                        }

                        // Renderable
                        {
                            sk_sp<GrTexture> tex;
                            if (SkBackingFit::kApprox == fit) {
                                tex = resourceProvider->createApproxTexture(
                                        dims, format, GrTextureType::k2D, GrRenderable::kYes,
                                        numSamples, GrProtected::kNo);
                            } else {
                                tex = resourceProvider->createTexture(
                                        dims, format, GrTextureType::k2D, GrRenderable::kYes,
                                        numSamples, GrMipmapped::kNo, budgeted, GrProtected::kNo);
                            }

                            sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(
                                    format, dims, GrRenderable::kYes, numSamples, GrMipmapped::kNo,
                                    fit, budgeted, GrProtected::kNo);
                            REPORTER_ASSERT(reporter, SkToBool(tex) == SkToBool(proxy));
                            if (proxy) {
                                REPORTER_ASSERT(reporter, proxy->asRenderTargetProxy());
                                // This forces the proxy to compute and cache its
                                // pre-instantiation size guess. Later, when it is actually
                                // instantiated, it checks that the instantiated size is <= to
                                // the pre-computation. If the proxy never computed its
                                // pre-instantiation size then the check is skipped.
                                proxy->gpuMemorySize();

                                check_surface(reporter, proxy.get(), widthHeight, widthHeight,
                                              budgeted);
                                int supportedSamples =
                                        caps.getRenderTargetSampleCount(numSamples, format);
                                check_rendertarget(reporter, caps, resourceProvider,
                                                   proxy->asRenderTargetProxy(), supportedSamples,
                                                   fit, caps.maxWindowRectangles());
                            }
                        }

                        // Not renderable
                        {
                            sk_sp<GrTexture> tex;
                            if (SkBackingFit::kApprox == fit) {
                                tex = resourceProvider->createApproxTexture(
                                        dims, format, GrTextureType::k2D, GrRenderable::kNo,
                                        numSamples, GrProtected::kNo);
                            } else {
                                tex = resourceProvider->createTexture(
                                        dims, format, GrTextureType::k2D, GrRenderable::kNo,
                                        numSamples, GrMipmapped::kNo, budgeted, GrProtected::kNo);
                            }

                            sk_sp<GrTextureProxy> proxy(proxyProvider->createProxy(
                                    format, dims, GrRenderable::kNo, numSamples, GrMipmapped::kNo,
                                    fit, budgeted, GrProtected::kNo));
                            REPORTER_ASSERT(reporter, SkToBool(tex) == SkToBool(proxy));
                            if (proxy) {
                                // This forces the proxy to compute and cache its
                                // pre-instantiation size guess. Later, when it is actually
                                // instantiated, it checks that the instantiated size is <= to
                                // the pre-computation. If the proxy never computed its
                                // pre-instantiation size then the check is skipped.
                                proxy->gpuMemorySize();

                                check_surface(reporter, proxy.get(), widthHeight, widthHeight,
                                              budgeted);
                                check_texture(reporter, resourceProvider, proxy->asTextureProxy(),
                                              fit);
                            }
                        }

                        attempt++;
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedProxyTest, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
    GrGpu* gpu = direct->priv().getGpu();
    const GrCaps& caps = *direct->priv().caps();

    static const int kWidthHeight = 100;

    for (auto colorType :
         {kAlpha_8_SkColorType, kRGBA_8888_SkColorType, kRGBA_1010102_SkColorType}) {
        GrColorType grColorType = SkColorTypeToGrColorType(colorType);

        // External on-screen render target.
        // Tests wrapBackendRenderTarget with a GrBackendRenderTarget
        // Our test-only function that creates a backend render target doesn't currently support
        // sample counts :(.
        if (direct->colorTypeSupportedAsSurface(colorType)) {
            GrBackendRenderTarget backendRT = gpu->createTestingOnlyBackendRenderTarget(
                    {kWidthHeight, kWidthHeight}, grColorType);
            sk_sp<GrSurfaceProxy> sProxy(
                    proxyProvider->wrapBackendRenderTarget(backendRT, nullptr));
            check_surface(reporter, sProxy.get(), kWidthHeight, kWidthHeight, SkBudgeted::kNo);
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

#ifdef SK_GL
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
                        proxyProvider->wrapBackendRenderTarget(backendRT, nullptr));
                check_surface(reporter, sProxy.get(), kWidthHeight, kWidthHeight, SkBudgeted::kNo);
                check_rendertarget(reporter, caps, resourceProvider, sProxy->asRenderTargetProxy(),
                                   supportedNumSamples, SkBackingFit::kExact, 0);
            }
#endif

            // Tests wrapBackendTexture that is only renderable
            {
                auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(direct,
                                                                                kWidthHeight,
                                                                                kWidthHeight,
                                                                                colorType,
                                                                                GrMipmapped::kNo,
                                                                                GrRenderable::kYes);
                if (!mbet) {
                    ERRORF(reporter,
                           "Could not create renderable backend texture of color type %d",
                           colorType);
                    continue;
                }
                sk_sp<GrSurfaceProxy> sProxy = proxyProvider->wrapRenderableBackendTexture(
                        mbet->texture(), supportedNumSamples, kBorrow_GrWrapOwnership,
                        GrWrapCacheable::kNo, nullptr);
                if (!sProxy) {
                    ERRORF(reporter, "wrapRenderableBackendTexture failed");
                    continue;
                }

                check_surface(reporter, sProxy.get(), kWidthHeight, kWidthHeight, SkBudgeted::kNo);
                check_rendertarget(reporter, caps, resourceProvider, sProxy->asRenderTargetProxy(),
                                   supportedNumSamples, SkBackingFit::kExact,
                                   caps.maxWindowRectangles());
            }

            {
                // Tests wrapBackendTexture that is only textureable
                auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(direct,
                                                                                kWidthHeight,
                                                                                kWidthHeight,
                                                                                colorType,
                                                                                GrMipmapped::kNo,
                                                                                GrRenderable::kNo);
                if (!mbet) {
                    ERRORF(reporter,
                           "Could not create non-renderable backend texture of color type %d",
                           colorType);
                    continue;
                }
                sk_sp<GrSurfaceProxy> sProxy = proxyProvider->wrapBackendTexture(
                        mbet->texture(), kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                        kRead_GrIOType, mbet->refCountedCallback());
                if (!sProxy) {
                    ERRORF(reporter, "wrapBackendTexture failed");
                    continue;
                }

                check_surface(reporter, sProxy.get(), kWidthHeight, kWidthHeight, SkBudgeted::kNo);
                check_texture(reporter, resourceProvider, sProxy->asTextureProxy(),
                              SkBackingFit::kExact);
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ZeroSizedProxyTest, reporter, ctxInfo) {
    auto direct = ctxInfo.directContext();
    GrProxyProvider* provider = direct->priv().proxyProvider();

    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto fit : { SkBackingFit::kExact, SkBackingFit::kApprox }) {
            for (int width : { 0, 100 }) {
                for (int height : { 0, 100}) {
                    if (width && height) {
                        continue; // not zero-sized
                    }

                    const GrBackendFormat format =
                            direct->priv().caps()->getDefaultBackendFormat(
                                GrColorType::kRGBA_8888,
                                renderable);

                    sk_sp<GrTextureProxy> proxy = provider->createProxy(
                            format, {width, height}, renderable, 1, GrMipmapped::kNo, fit,
                            SkBudgeted::kNo, GrProtected::kNo);
                    REPORTER_ASSERT(reporter, !proxy);
                }
            }
        }
    }
}
