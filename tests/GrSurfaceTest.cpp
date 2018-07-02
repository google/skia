/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "GrTexture.h"
#include "SkMipMap.h"
#include "Test.h"

// Tests that GrSurface::asTexture(), GrSurface::asRenderTarget(), and static upcasting of texture
// and render targets to GrSurface all work as expected.
DEF_GPUTEST_FOR_NULLGL_CONTEXT(GrSurface, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    auto resourceProvider = context->contextPriv().resourceProvider();
    GrGpu* gpu = context->contextPriv().getGpu();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 256;
    desc.fHeight = 256;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fSampleCnt = 1;
    sk_sp<GrSurface> texRT1 = resourceProvider->createTexture(desc, SkBudgeted::kNo);

    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    texRT1->asTexture());
    REPORTER_ASSERT(reporter, texRT1->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT1->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT1->asTexture()));

    desc.fFlags = kNone_GrSurfaceFlags;
    sk_sp<GrTexture> tex1 = resourceProvider->createTexture(desc, SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, nullptr == tex1->asRenderTarget());
    REPORTER_ASSERT(reporter, tex1.get() == tex1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(tex1.get()) == tex1->asTexture());

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
        nullptr, 256, 256, kRGBA_8888_GrPixelConfig, false, GrMipMapped::kNo);

    sk_sp<GrSurface> texRT2 =
            resourceProvider->wrapRenderableBackendTexture(backendTex, 1, kBorrow_GrWrapOwnership);

    REPORTER_ASSERT(reporter, texRT2.get() == texRT2->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT2.get() == texRT2->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT2->asRenderTarget()) ==
                    texRT2->asTexture());
    REPORTER_ASSERT(reporter, texRT2->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT2->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT2->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT2->asTexture()));

    gpu->deleteTestingOnlyBackendTexture(backendTex);
}

// This test checks that the isConfigTexturable and isConfigRenderable are
// consistent with createTexture's result.
DEF_GPUTEST_FOR_ALL_CONTEXTS(GrSurfaceRenderability, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    GrResourceProvider* resourceProvider = context->contextPriv().resourceProvider();
    const GrCaps* caps = context->contextPriv().caps();

    GrPixelConfig configs[] = {
        kUnknown_GrPixelConfig,
        kAlpha_8_GrPixelConfig,
        kAlpha_8_as_Alpha_GrPixelConfig,
        kAlpha_8_as_Red_GrPixelConfig,
        kGray_8_GrPixelConfig,
        kGray_8_as_Lum_GrPixelConfig,
        kGray_8_as_Red_GrPixelConfig,
        kRGB_565_GrPixelConfig,
        kRGBA_4444_GrPixelConfig,
        kRGBA_8888_GrPixelConfig,
        kRGB_888_GrPixelConfig,
        kBGRA_8888_GrPixelConfig,
        kSRGBA_8888_GrPixelConfig,
        kSBGRA_8888_GrPixelConfig,
        kRGBA_1010102_GrPixelConfig,
        kRGBA_float_GrPixelConfig,
        kRG_float_GrPixelConfig,
        kAlpha_half_GrPixelConfig,
        kAlpha_half_as_Red_GrPixelConfig,
        kRGBA_half_GrPixelConfig,
    };
    GR_STATIC_ASSERT(kGrPixelConfigCnt == SK_ARRAY_COUNT(configs));

    GrSurfaceDesc desc;
    desc.fWidth = 64;
    desc.fHeight = 64;

    for (GrPixelConfig config : configs) {
        for (GrSurfaceOrigin origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin }) {
            desc.fFlags = kNone_GrSurfaceFlags;
            desc.fConfig = config;
            desc.fSampleCnt = 1;

            sk_sp<GrSurface> tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
            bool ict = caps->isConfigTexturable(desc.fConfig);
            REPORTER_ASSERT(reporter, SkToBool(tex) == ict,
                            "config:%d, tex:%d, isConfigTexturable:%d", config, SkToBool(tex), ict);

            sk_sp<GrTextureProxy> proxy =
                    proxyProvider->createMipMapProxy(desc, origin, SkBudgeted::kNo);
            REPORTER_ASSERT(reporter, SkToBool(proxy.get()) ==
                            (caps->isConfigTexturable(desc.fConfig) &&
                             caps->mipMapSupport()));

            desc.fFlags = kRenderTarget_GrSurfaceFlag;
            tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
            bool isRenderable = caps->isConfigRenderable(config);
            REPORTER_ASSERT(reporter, SkToBool(tex) == isRenderable,
                            "config:%d, tex:%d, isRenderable:%d", config, SkToBool(tex),
                            isRenderable);

            desc.fSampleCnt = 2;
            tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
            isRenderable = SkToBool(caps->getRenderTargetSampleCount(2, config));
            REPORTER_ASSERT(reporter, SkToBool(tex) == isRenderable,
                            "config:%d, tex:%d, isRenderable:%d", config, SkToBool(tex),
                            isRenderable);
        }
    }
}

#include "GrDrawingManager.h"
#include "GrSurfaceProxy.h"
#include "GrTextureContext.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(InitialTextureClear, reporter, context_info) {
    static constexpr int kSize = 100;
    GrSurfaceDesc desc;
    desc.fWidth = desc.fHeight = kSize;
    std::unique_ptr<uint32_t[]> data(new uint32_t[kSize * kSize]);

    GrContext* context = context_info.grContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    for (int c = 0; c <= kLast_GrPixelConfig; ++c) {
        desc.fConfig = static_cast<GrPixelConfig>(c);
        if (!context->contextPriv().caps()->isConfigTexturable(desc.fConfig)) {
            continue;
        }
        desc.fFlags = kPerformInitialClear_GrSurfaceFlag;
        for (bool rt : {false, true}) {
            if (rt && !context->contextPriv().caps()->isConfigRenderable(desc.fConfig)) {
                continue;
            }
            desc.fFlags |= rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
            for (GrSurfaceOrigin origin :
                 {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
                for (auto fit : { SkBackingFit::kApprox, SkBackingFit::kExact }) {
                    // Try directly creating the texture.
                    // Do this twice in an attempt to hit the cache on the second time through.
                    for (int i = 0; i < 2; ++i) {
                        sk_sp<GrTextureProxy> proxy = proxyProvider->createInstantiatedProxy(
                                desc, origin, fit, SkBudgeted::kYes);
                        if (!proxy) {
                            continue;
                        }
                        auto texCtx = context->contextPriv().makeWrappedSurfaceContext(
                                std::move(proxy));
                        SkImageInfo info = SkImageInfo::Make(
                                kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                        memset(data.get(), 0xAB, kSize * kSize * sizeof(uint32_t));
                        if (texCtx->readPixels(info, data.get(), 0, 0, 0)) {
                            uint32_t cmp = GrPixelConfigIsOpaque(desc.fConfig) ? 0xFF000000 : 0;
                            for (int i = 0; i < kSize * kSize; ++i) {
                                if (cmp != data.get()[i]) {
                                    ERRORF(reporter, "Failed on config %d", desc.fConfig);
                                    break;
                                }
                            }
                        }
                        memset(data.get(), 0xBC, kSize * kSize * sizeof(uint32_t));
                        // Here we overwrite the texture so that the second time through we
                        // test against recycling without reclearing.
                        if (0 == i) {
                            texCtx->writePixels(info, data.get(), 0, 0, 0);
                        }
                    }
                    context->contextPriv().purgeAllUnlockedResources_ForTesting();

                    // Try creating the texture as a deferred proxy.
                    for (int i = 0; i < 2; ++i) {
                        auto surfCtx = context->contextPriv().makeDeferredSurfaceContext(
                                desc, origin, GrMipMapped::kNo, fit, SkBudgeted::kYes);
                        if (!surfCtx) {
                            continue;
                        }
                        SkImageInfo info = SkImageInfo::Make(
                                kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                        memset(data.get(), 0xAB, kSize * kSize * sizeof(uint32_t));
                        if (surfCtx->readPixels(info, data.get(), 0, 0, 0)) {
                            uint32_t cmp = GrPixelConfigIsOpaque(desc.fConfig) ? 0xFF000000 : 0;
                            for (int i = 0; i < kSize * kSize; ++i) {
                                if (cmp != data.get()[i]) {
                                    ERRORF(reporter, "Failed on config %d", desc.fConfig);
                                    break;
                                }
                            }
                        }
                        // Here we overwrite the texture so that the second time through we
                        // test against recycling without reclearing.
                        if (0 == i) {
                            surfCtx->writePixels(info, data.get(), 0, 0, 0);
                        }
                    }
                    context->contextPriv().purgeAllUnlockedResources_ForTesting();
                }
            }
        }
    }
}
