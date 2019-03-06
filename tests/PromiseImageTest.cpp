/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrTexture.h"
#include "SkImage_Gpu.h"
#include "SkPromiseImageTexture.h"

using namespace sk_gpu_test;

struct PromiseTextureChecker {
    // shared indicates whether the backend texture is used to fulfill more than one promise
    // image.
    explicit PromiseTextureChecker(const GrBackendTexture& tex, skiatest::Reporter* reporter,
                                   bool shared)
            : fTexture(SkPromiseImageTexture::Make(tex))
            , fReporter(reporter)
            , fShared(shared)
            , fFulfillCount(0)
            , fReleaseCount(0)
            , fDoneCount(0) {}
    sk_sp<SkPromiseImageTexture> fTexture;
    skiatest::Reporter* fReporter;
    bool fShared;
    int fFulfillCount;
    int fReleaseCount;
    int fDoneCount;

    /**
     * Releases the SkPromiseImageTexture. Used to test that cached GrTexture representations
     * in the cache are freed.
     */
    void releaseTexture() { fTexture.reset(); }

    SkTArray<GrUniqueKey> uniqueKeys() const {
        return fTexture->testingOnly_uniqueKeysToInvalidate();
    }

    static sk_sp<SkPromiseImageTexture> Fulfill(void* self) {
        auto checker = static_cast<PromiseTextureChecker*>(self);
        checker->fFulfillCount++;
        return checker->fTexture;
    }
    static void Release(void* self) {
        auto checker = static_cast<PromiseTextureChecker*>(self);
        checker->fReleaseCount++;
        if (!checker->fShared) {
            // This is only used in a single threaded fashion with a single promise image. So
            // every fulfill should be balanced by a release before the next fulfill.
            REPORTER_ASSERT(checker->fReporter, checker->fReleaseCount == checker->fFulfillCount);
        }
    }
    static void Done(void* self) {
        static_cast<PromiseTextureChecker*>(self)->fDoneCount++;
    }
};

enum class ReleaseBalanceExpectation {
    kBalanced,
    kAllUnbalanced,
    kUnbalancedByOne,
};

enum class DoneBalanceExpectation {
    kBalanced,
    kAllUnbalanced,
    kUnknown,
    kUnbalancedByOne,
    kBalancedOrOffByOne,
};

static void check_fulfill_and_release_cnts(skiatest::Reporter* reporter,
                                           const PromiseTextureChecker& promiseChecker,
                                           int expectedFulfillCnt,
                                           ReleaseBalanceExpectation releaseBalanceExpecation,
                                           DoneBalanceExpectation doneBalanceExpecation) {
    REPORTER_ASSERT(reporter, promiseChecker.fFulfillCount == expectedFulfillCnt);
    if (!expectedFulfillCnt) {
        // Release and Done should only ever be called after Fulfill.
        REPORTER_ASSERT(reporter, !promiseChecker.fReleaseCount);
        REPORTER_ASSERT(reporter, !promiseChecker.fDoneCount);
        return;
    }
    int releaseDiff = promiseChecker.fFulfillCount - promiseChecker.fReleaseCount;
    switch (releaseBalanceExpecation) {
        case ReleaseBalanceExpectation::kBalanced:
            REPORTER_ASSERT(reporter, !releaseDiff);
            break;
        case ReleaseBalanceExpectation::kAllUnbalanced:
            REPORTER_ASSERT(reporter, releaseDiff == promiseChecker.fFulfillCount);
            break;
        case ReleaseBalanceExpectation::kUnbalancedByOne:
            REPORTER_ASSERT(reporter, releaseDiff == 1);
            break;
    }
    int doneDiff = promiseChecker.fFulfillCount - promiseChecker.fDoneCount;
    switch (doneBalanceExpecation) {
        case DoneBalanceExpectation::kBalanced:
            REPORTER_ASSERT(reporter, !doneDiff);
            break;
        case DoneBalanceExpectation::kAllUnbalanced:
            REPORTER_ASSERT(reporter, doneDiff == promiseChecker.fFulfillCount);
            break;
        case DoneBalanceExpectation::kUnknown:
            REPORTER_ASSERT(reporter, doneDiff >= 0 && doneDiff <= promiseChecker.fFulfillCount);
            break;
        case DoneBalanceExpectation::kUnbalancedByOne:
            REPORTER_ASSERT(reporter, doneDiff == 1);
            break;
        case DoneBalanceExpectation::kBalancedOrOffByOne:
            REPORTER_ASSERT(reporter, doneDiff == 0 || doneDiff == 1);
            break;
    }
}

static void check_unfulfilled(const PromiseTextureChecker& promiseChecker,
                              skiatest::Reporter* reporter) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, 0,
                                   ReleaseBalanceExpectation::kBalanced,
                                   DoneBalanceExpectation::kBalanced);
}

static void check_only_fulfilled(skiatest::Reporter* reporter,
                                 const PromiseTextureChecker& promiseChecker,
                                 int expectedFulfillCnt = 1) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kAllUnbalanced,
                                   DoneBalanceExpectation::kAllUnbalanced);
}

static void check_all_flushed_but_not_synced(skiatest::Reporter* reporter,
                                             const PromiseTextureChecker& promiseChecker,
                                             GrBackendApi api,
                                             int expectedFulfillCnt = 1) {
    DoneBalanceExpectation doneBalanceExpectation = DoneBalanceExpectation::kBalanced;
    // On Vulkan Done isn't guaranteed to be called until a sync has occurred.
    if (api == GrBackendApi::kVulkan) {
        doneBalanceExpectation = expectedFulfillCnt == 1
                                         ? DoneBalanceExpectation::kBalancedOrOffByOne
                                         : DoneBalanceExpectation::kUnknown;
    }
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kBalanced, doneBalanceExpectation);
}

static void check_all_done(skiatest::Reporter* reporter,
                           const PromiseTextureChecker& promiseChecker,
                           int expectedFulfillCnt = 1) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kBalanced,
                                   DoneBalanceExpectation::kBalanced);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTest, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->priv().getGpu();

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, true, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrBackendFormat backendFormat = backendTex.getBackendFormat();
    REPORTER_ASSERT(reporter, backendFormat.isValid());

    PromiseTextureChecker promiseChecker(backendTex, reporter, false);
    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(
            SkImage_Gpu::MakePromiseTexture(
                    ctx, backendFormat, kWidth, kHeight,
                    GrMipMapped::kNo, texOrigin,
                    kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                    nullptr,
                    PromiseTextureChecker::Fulfill,
                    PromiseTextureChecker::Release,
                    PromiseTextureChecker::Done,
                    &promiseChecker,
                    SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    canvas->drawImage(refImg, 0, 0);
    check_unfulfilled(promiseChecker, reporter);

    surface->flush();
    // We still own the image so we should not have called Release or Done.
    check_only_fulfilled(reporter, promiseChecker);

    gpu->testingOnly_flushGpuAndSync();
    check_only_fulfilled(reporter, promiseChecker);

    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg, 0, 0);

    surface->flush();

    gpu->testingOnly_flushGpuAndSync();
    // Image should still be fulfilled from the first time we drew/flushed it.
    check_only_fulfilled(reporter, promiseChecker);

    canvas->drawImage(refImg, 0, 0);
    surface->flush();
    check_only_fulfilled(reporter, promiseChecker);

    canvas->drawImage(refImg, 0, 0);
    refImg.reset();
    // We no longer own the image but the last draw is still unflushed.
    check_only_fulfilled(reporter, promiseChecker);

    surface->flush();
    // Flushing should have called Release. Depending on the backend and timing it may have called
    // done.
    check_all_flushed_but_not_synced(reporter, promiseChecker, ctx->backend());
    gpu->testingOnly_flushGpuAndSync();
    // Now Done should definitely have been called.
    check_all_done(reporter, promiseChecker);

    gpu->deleteTestingOnlyBackendTexture(backendTex);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureReuseDifferentConfig, reporter, ctxInfo) {
    // Try making two promise SkImages backed by the same texture but with different configs.
    // This will only be testable on backends where a single texture format (8bit red unorm) can
    // be used for alpha and gray image color types.

    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->priv().getGpu();

    GrBackendTexture backendTex1 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kGray_8, false, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex1.isValid());

    GrBackendTexture backendTex2 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kAlpha_8, false, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex2.isValid());
    if (backendTex1.getBackendFormat() != backendTex2.getBackendFormat()) {
        gpu->deleteTestingOnlyBackendTexture(backendTex1);
        return;
    }
    // We only needed this texture to check that alpha and gray color types use the same format.
    gpu->deleteTestingOnlyBackendTexture(backendTex2);

    SkImageInfo info =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    PromiseTextureChecker promiseChecker(backendTex1, reporter, true);
    sk_sp<SkImage> alphaImg(SkImage_Gpu::MakePromiseTexture(
            ctx, backendTex1.getBackendFormat(), kWidth, kHeight, GrMipMapped::kNo,
            kTopLeft_GrSurfaceOrigin, kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr,
            PromiseTextureChecker::Fulfill, PromiseTextureChecker::Release,
            PromiseTextureChecker::Done, &promiseChecker,
            SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew));
    REPORTER_ASSERT(reporter, alphaImg);

    sk_sp<SkImage> grayImg(SkImage_Gpu::MakePromiseTexture(
            ctx, backendTex1.getBackendFormat(), kWidth, kHeight, GrMipMapped::kNo,
            kBottomLeft_GrSurfaceOrigin, kGray_8_SkColorType, kOpaque_SkAlphaType, nullptr,
            PromiseTextureChecker::Fulfill, PromiseTextureChecker::Release,
            PromiseTextureChecker::Done, &promiseChecker,
            SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew));
    REPORTER_ASSERT(reporter, grayImg);

    canvas->drawImage(alphaImg, 0, 0);
    canvas->drawImage(grayImg, 1, 1);
    surface->flush();
    gpu->testingOnly_flushGpuAndSync();
    check_only_fulfilled(reporter, promiseChecker, 2);

    // Because they use different configs, each image should have created a different GrTexture
    // and they both should still be cached.
    ctx->priv().getResourceCache()->purgeAsNeeded();

    auto keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 2);
    for (const auto& key : keys) {
        auto surf = ctx->priv().resourceProvider()->findByUniqueKey<GrSurface>(key);
        REPORTER_ASSERT(reporter, surf && surf->asTexture());
        if (surf && surf->asTexture()) {
            REPORTER_ASSERT(reporter,
                            !GrBackendTexture::TestingOnly_Equals(
                                    backendTex1, surf->asTexture()->getBackendTexture()));
        }
    }

    // Invalidate the backing texture, this should invalidate the keys.
    promiseChecker.releaseTexture();
    ctx->priv().getResourceCache()->purgeAsNeeded();

    for (const auto& key : keys) {
        auto surf = ctx->priv().resourceProvider()->findByUniqueKey<GrSurface>(key);
        REPORTER_ASSERT(reporter, !surf);
    }
    alphaImg.reset();
    check_fulfill_and_release_cnts(reporter, promiseChecker, 2,
                                   ReleaseBalanceExpectation::kUnbalancedByOne,
                                   DoneBalanceExpectation::kUnbalancedByOne);
    grayImg.reset();
    check_all_done(reporter, promiseChecker, 2);
    gpu->deleteTestingOnlyBackendTexture(backendTex1);
}

DEF_GPUTEST(PromiseImageTextureShutdown, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    // Different ways of killing contexts.
    using DeathFn = std::function<void(sk_gpu_test::GrContextFactory*, GrContext*)>;
    DeathFn destroy = [](sk_gpu_test::GrContextFactory* factory, GrContext* context) {
        factory->destroyContexts();
    };
    DeathFn abandon = [](sk_gpu_test::GrContextFactory* factory, GrContext* context) {
        context->abandonContext();
    };
    DeathFn releaseResourcesAndAbandon = [](sk_gpu_test::GrContextFactory* factory,
                                            GrContext* context) {
        context->releaseResourcesAndAbandonContext();
    };

    for (int type = 0; type < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++type) {
        auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(type);
        // These tests are difficult to get working with Vulkan. See http://skbug.com/8705
        // and http://skbug.com/8275
        GrBackendApi api = sk_gpu_test::GrContextFactory::ContextTypeBackend(contextType);
        if (api == GrBackendApi::kVulkan) {
            continue;
        }
        DeathFn contextKillers[] = {destroy, abandon, releaseResourcesAndAbandon};
        for (auto contextDeath : contextKillers) {
            sk_gpu_test::GrContextFactory factory;
            auto ctx = factory.get(contextType);
            if (!ctx) {
                continue;
            }
            GrGpu* gpu = ctx->priv().getGpu();

            GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
                    nullptr, kWidth, kHeight, GrColorType::kAlpha_8, false, GrMipMapped::kNo);
            REPORTER_ASSERT(reporter, backendTex.isValid());

            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);
            sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
            SkCanvas* canvas = surface->getCanvas();

            PromiseTextureChecker promiseChecker(backendTex, reporter, false);
            sk_sp<SkImage> image(SkImage_Gpu::MakePromiseTexture(
                    ctx, backendTex.getBackendFormat(), kWidth, kHeight, GrMipMapped::kNo,
                    kTopLeft_GrSurfaceOrigin, kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr,
                    PromiseTextureChecker::Fulfill, PromiseTextureChecker::Release,
                    PromiseTextureChecker::Done, &promiseChecker,
                    SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew));
            REPORTER_ASSERT(reporter, image);

            canvas->drawImage(image, 0, 0);
            image.reset();
            // If the surface still holds a ref to the context then the factory will not be able
            // to destroy the context (and instead will release-all-and-abandon).
            surface.reset();

            ctx->flush();
            contextDeath(&factory, ctx);

            check_all_done(reporter, promiseChecker);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureFullCache, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->priv().getGpu();

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kAlpha_8, false, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    SkImageInfo info =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    PromiseTextureChecker promiseChecker(backendTex, reporter, false);
    sk_sp<SkImage> image(SkImage_Gpu::MakePromiseTexture(
            ctx, backendTex.getBackendFormat(), kWidth, kHeight, GrMipMapped::kNo,
            kTopLeft_GrSurfaceOrigin, kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr,
            PromiseTextureChecker::Fulfill, PromiseTextureChecker::Release,
            PromiseTextureChecker::Done, &promiseChecker,
            SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew));
    REPORTER_ASSERT(reporter, image);

    // Make the cache full. This tests that we don't preemptively purge cached textures for
    // fulfillment due to cache pressure.
    static constexpr int kMaxResources = 10;
    static constexpr int kMaxBytes = 100;
    ctx->setResourceCacheLimits(kMaxResources, kMaxBytes);
    sk_sp<GrTexture> textures[2 * kMaxResources];
    for (int i = 0; i < 2 * kMaxResources; ++i) {
        GrSurfaceDesc desc;
        desc.fConfig = kRGBA_8888_GrPixelConfig;
        desc.fWidth = desc.fHeight = 100;
        textures[i] = ctx->priv().resourceProvider()->createTexture(desc, SkBudgeted::kYes);
        REPORTER_ASSERT(reporter, textures[i]);
    }

    // Relying on the asserts in the promiseImageChecker to ensure that fulfills and releases are
    // properly ordered.
    canvas->drawImage(image, 0, 0);
    surface->flush();
    canvas->drawImage(image, 1, 0);
    surface->flush();
    canvas->drawImage(image, 2, 0);
    surface->flush();
    canvas->drawImage(image, 3, 0);
    surface->flush();
    canvas->drawImage(image, 4, 0);
    surface->flush();
    canvas->drawImage(image, 5, 0);
    surface->flush();
    // Must call this to ensure that all callbacks are performed before the checker is destroyed.
    gpu->testingOnly_flushGpuAndSync();
    gpu->deleteTestingOnlyBackendTexture(backendTex);
}
