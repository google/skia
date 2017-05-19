/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "GrTexture.h"
#include "GrSurfacePriv.h"
#include "Test.h"

// Tests that GrSurface::asTexture(), GrSurface::asRenderTarget(), and static upcasting of texture
// and render targets to GrSurface all work as expected.
DEF_GPUTEST_FOR_NULLGL_CONTEXT(GrSurface, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 256;
    desc.fHeight = 256;
    desc.fSampleCnt = 0;
    sk_sp<GrSurface> texRT1 = context->resourceProvider()->createTexture(desc, SkBudgeted::kNo);

    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT1.get() == texRT1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    texRT1->asTexture());
    REPORTER_ASSERT(reporter, texRT1->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT1->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT1->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT1->asTexture()));

    desc.fFlags = kNone_GrSurfaceFlags;
    sk_sp<GrTexture> tex1 = context->resourceProvider()->createTexture(desc, SkBudgeted::kNo);
    REPORTER_ASSERT(reporter, nullptr == tex1->asRenderTarget());
    REPORTER_ASSERT(reporter, tex1.get() == tex1->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(tex1.get()) == tex1->asTexture());

    GrBackendObject backendTexHandle = context->getGpu()->createTestingOnlyBackendTexture(
        nullptr, 256, 256, kRGBA_8888_GrPixelConfig);
    GrBackendTexture backendTex = GrTest::CreateBackendTexture(context->contextPriv().getBackend(),
                                                               256,
                                                               256,
                                                               kRGBA_8888_GrPixelConfig,
                                                               backendTexHandle);

    sk_sp<GrSurface> texRT2 = context->resourceProvider()->wrapBackendTexture(
        backendTex, kTopLeft_GrSurfaceOrigin, kRenderTarget_GrBackendTextureFlag, 0,
        kBorrow_GrWrapOwnership);

    REPORTER_ASSERT(reporter, texRT2.get() == texRT2->asRenderTarget());
    REPORTER_ASSERT(reporter, texRT2.get() == texRT2->asTexture());
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT2->asRenderTarget()) ==
                    texRT2->asTexture());
    REPORTER_ASSERT(reporter, texRT2->asRenderTarget() ==
                    static_cast<GrSurface*>(texRT2->asTexture()));
    REPORTER_ASSERT(reporter, static_cast<GrSurface*>(texRT2->asRenderTarget()) ==
                    static_cast<GrSurface*>(texRT2->asTexture()));

    context->getGpu()->deleteTestingOnlyBackendTexture(backendTexHandle);
}

#include "GrDrawingManager.h"
#include "GrSurfaceProxy.h"
#include "GrTextureContext.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureInitialClear, reporter, context_info) {
    static constexpr int kSize = 100;
    GrSurfaceDesc desc;
    desc.fWidth = desc.fHeight = kSize;
    uint32_t data[kSize * kSize];
    GrContext* context = context_info.grContext();
    for (int c = 0; c <= kLast_GrPixelConfig; ++c) {
        desc.fConfig = static_cast<GrPixelConfig>(c);
        if (!context_info.grContext()->caps()->isConfigTexturable(desc.fConfig)) {
            continue;
        }
        desc.fFlags = kPerformInitialClear_GrSurfaceFlag;
        for (bool rt : {false, true}) {
            if (rt && !context->caps()->isConfigRenderable(desc.fConfig, false)) {
                continue;
            }
            desc.fFlags |= rt ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
            for (bool mipped : {false, true}) {
                desc.fIsMipMapped = mipped;
                for (GrSurfaceOrigin origin :
                     {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
                    desc.fOrigin = origin;
                    for (bool approx : {false, true}) {
                        auto resourceProvider = context->resourceProvider();
                        // Try directly creating the texture.
                        // Do this twice in an attempt to hit the cache on the second time through.
                        for (int i = 0; i < 2; ++i) {
                            auto tex =
                                    approx ? sk_sp<GrTexture>(
                                                     resourceProvider->createApproxTexture(desc, 0))
                                           : resourceProvider->createTexture(desc,
                                                                             SkBudgeted::kYes);
                            // We should fail to create the texture if it is compressed (since the
                            // initial clear flag is not supported) and succeed otherwise.
                            REPORTER_ASSERT(reporter, SkToBool(tex) == !GrPixelConfigIsCompressed(
                                                                               desc.fConfig));
                            if (!tex) {
                                continue;
                            }
                            auto proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));
                            auto tctx = context->contextPriv().makeWrappedSurfaceContext(
                                    std::move(proxy), nullptr);
                            SkImageInfo info = SkImageInfo::Make(
                                    kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                            memset(data, 0xAB, kSize * kSize * sizeof(uint32_t));
                            if (tctx->readPixels(info, data, 0, 0, 0)) {
                                uint32_t cmp = GrPixelConfigIsOpaque(desc.fConfig) ? 0xFF000000 : 0;
                                for (int i = 0; i < kSize * kSize; ++i) {
                                    if (cmp != data[i]) {
                                        ERRORF(reporter, "Failed on conifg %d", desc.fConfig);
                                        break;
                                    }
                                }
                            }
                        }
                        context->purgeAllUnlockedResources();
                        // Try creating the texture as a deferred proxy.
                        for (int i = 0; i < 2; ++i) {
                            auto surfCtx = context->contextPriv().makeDeferredSurfaceContext(
                                    desc, approx ? SkBackingFit::kApprox : SkBackingFit::kExact,
                                    SkBudgeted::kYes);
                            // We should fail to create the texture if it is compressed (since the
                            // initial clear flag is not supported) and succeed otherwise.
                            REPORTER_ASSERT(
                                    reporter,
                                    SkToBool(surfCtx) == !GrPixelConfigIsCompressed(desc.fConfig));
                            if (!surfCtx) {
                                continue;
                            }
                            SkImageInfo info = SkImageInfo::Make(
                                    kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                            memset(data, 0xAB, kSize * kSize * sizeof(uint32_t));
                            if (surfCtx->readPixels(info, data, 0, 0, 0)) {
                                uint32_t cmp = GrPixelConfigIsOpaque(desc.fConfig) ? 0xFF000000 : 0;
                                for (int i = 0; i < kSize * kSize; ++i) {
                                    if (cmp != data[i]) {
                                        ERRORF(reporter, "Failed on conifg %d", desc.fConfig);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif
