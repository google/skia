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
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "GrTexture.h"
#include "GrSurfacePriv.h"
#include "SkMipMap.h"
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

    sk_sp<GrSurface> texRT2 = context->resourceProvider()->wrapRenderableBackendTexture(
            backendTex, kTopLeft_GrSurfaceOrigin, 0, kBorrow_GrWrapOwnership);

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

// This test checks that the isConfigTexturable and isConfigRenderable are
// consistent with createTexture's result.
DEF_GPUTEST_FOR_ALL_CONTEXTS(GrSurfaceRenderability, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrResourceProvider* resourceProvider = context->resourceProvider();
    const GrCaps* caps = context->caps();

    GrPixelConfig configs[] = {
        kUnknown_GrPixelConfig,
        kAlpha_8_GrPixelConfig,
        kGray_8_GrPixelConfig,
        kRGB_565_GrPixelConfig,
        kRGBA_4444_GrPixelConfig,
        kRGBA_8888_GrPixelConfig,
        kBGRA_8888_GrPixelConfig,
        kSRGBA_8888_GrPixelConfig,
        kSBGRA_8888_GrPixelConfig,
        kRGBA_8888_sint_GrPixelConfig,
        kRGBA_float_GrPixelConfig,
        kRG_float_GrPixelConfig,
        kAlpha_half_GrPixelConfig,
        kRGBA_half_GrPixelConfig,
    };
    SkASSERT(kGrPixelConfigCnt == SK_ARRAY_COUNT(configs));

    GrSurfaceDesc desc;
    desc.fWidth = 64;
    desc.fHeight = 64;

    // Enough space for the first mip of our largest pixel config
    const size_t pixelBufferSize = desc.fWidth * desc.fHeight *
                                   GrBytesPerPixel(kRGBA_float_GrPixelConfig);
    std::unique_ptr<char[]> pixelData(new char[pixelBufferSize]);
    memset(pixelData.get(), 0, pixelBufferSize);

    // We re-use the same mip level objects (with updated pointers and rowBytes) for each config
    const int levelCount = SkMipMap::ComputeLevelCount(desc.fWidth, desc.fHeight) + 1;
    std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[levelCount]);

    for (GrPixelConfig config : configs) {
        for (GrSurfaceOrigin origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin }) {
            desc.fFlags = kNone_GrSurfaceFlags;
            desc.fOrigin = origin;
            desc.fSampleCnt = 0;
            desc.fConfig = config;

            sk_sp<GrSurface> tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
            REPORTER_ASSERT(reporter, SkToBool(tex.get()) == caps->isConfigTexturable(desc.fConfig));

            size_t rowBytes = desc.fWidth * GrBytesPerPixel(desc.fConfig);
            for (int i = 0; i < levelCount; ++i) {
                texels[i].fPixels = pixelData.get();
                texels[i].fRowBytes = rowBytes >> i;
            }
            sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferredMipMap(resourceProvider,
                                                                             desc, SkBudgeted::kNo,
                                                                             texels.get(),
                                                                             levelCount);
            REPORTER_ASSERT(reporter, SkToBool(proxy.get()) ==
                            (caps->isConfigTexturable(desc.fConfig) &&
                             caps->mipMapSupport() &&
                             !GrPixelConfigIsSint(desc.fConfig)));

            desc.fFlags = kRenderTarget_GrSurfaceFlag;
            tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
            REPORTER_ASSERT(reporter, SkToBool(tex.get()) == caps->isConfigRenderable(config, false));

            desc.fSampleCnt = 4;
            tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
            REPORTER_ASSERT(reporter, SkToBool(tex.get()) == caps->isConfigRenderable(config, true));
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
                            sk_sp<GrTexture> tex;
                            if (approx) {
                                tex = sk_sp<GrTexture>(
                                        resourceProvider->createApproxTexture(desc, 0));
                            } else {
                                tex = resourceProvider->createTexture(desc, SkBudgeted::kYes);
                            }
                            if (!tex) {
                                continue;
                            }
                            auto proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));
                            auto texCtx = context->contextPriv().makeWrappedSurfaceContext(
                                    std::move(proxy), nullptr);
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
                        context->purgeAllUnlockedResources();

                        // Try creating the texture as a deferred proxy.
                        for (int i = 0; i < 2; ++i) {
                            auto surfCtx = context->contextPriv().makeDeferredSurfaceContext(
                                    desc, approx ? SkBackingFit::kApprox : SkBackingFit::kExact,
                                    SkBudgeted::kYes);
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
                        context->purgeAllUnlockedResources();
                    }
                }
            }
        }
    }
}
#endif
