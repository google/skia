/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkExchange.h"
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
    GrBackendTexture fLastFulfilledTexture;

    /**
     * Replaces the backend texture that this checker will return from fulfill. Also, transfers
     * ownership of the previous PromiseImageTexture to the caller, if they want to control when
     * it is deleted. The default argument will remove the existing texture without installing a
     * valid replacement.
     */
    sk_sp<const SkPromiseImageTexture> replaceTexture(
            const GrBackendTexture& tex = GrBackendTexture()) {
        return skstd::exchange(fTexture, SkPromiseImageTexture::Make(tex));
    }

    SkTArray<GrUniqueKey> uniqueKeys() const {
        return fTexture->testingOnly_uniqueKeysToInvalidate();
    }

    static sk_sp<SkPromiseImageTexture> Fulfill(void* self) {
        auto checker = static_cast<PromiseTextureChecker*>(self);
        checker->fFulfillCount++;
        checker->fLastFulfilledTexture = checker->fTexture->backendTexture();
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

enum class ReleaseBalanceExpecation {
    kBalanced,
    kBalancedOrPlusOne,
    kAny
};

static bool check_fulfill_and_release_cnts(const PromiseTextureChecker& promiseChecker,
                                           ReleaseBalanceExpecation balanceExpecation,
                                           int expectedFulfillCnt,
                                           int expectedReleaseCnt,
                                           bool expectedRequired,
                                           int expectedDoneCnt,
                                           skiatest::Reporter* reporter) {
    bool result = true;
    int countDiff = promiseChecker.fFulfillCount - promiseChecker.fReleaseCount;
    // FulfillCount should always equal ReleaseCount or be at most one higher
    if (countDiff != 0) {
        if (balanceExpecation == ReleaseBalanceExpecation::kBalanced) {
            result = false;
            REPORTER_ASSERT(reporter, 0 == countDiff);
        } else if (countDiff != 1 &&
                   balanceExpecation == ReleaseBalanceExpecation::kBalancedOrPlusOne) {
            result = false;
            REPORTER_ASSERT(reporter, 0 == countDiff || 1 == countDiff);
        } else if (countDiff < 0) {
            result = false;
            REPORTER_ASSERT(reporter, countDiff >= 0);
        }
    }

    int fulfillDiff = expectedFulfillCnt - promiseChecker.fFulfillCount;
    REPORTER_ASSERT(reporter, fulfillDiff >= 0);
    if (fulfillDiff != 0) {
        if (expectedRequired) {
            result = false;
            REPORTER_ASSERT(reporter, expectedFulfillCnt == promiseChecker.fFulfillCount);
        } else if (fulfillDiff > 1) {
            result = false;
            REPORTER_ASSERT(reporter, fulfillDiff <= 1);
        }
    }

    int releaseDiff = expectedReleaseCnt - promiseChecker.fReleaseCount;
    REPORTER_ASSERT(reporter, releaseDiff >= 0);
    if (releaseDiff != 0) {
        if (expectedRequired) {
            result = false;
            REPORTER_ASSERT(reporter, expectedReleaseCnt == promiseChecker.fReleaseCount);
        } else if (releaseDiff > 1) {
            result = false;
            REPORTER_ASSERT(reporter, releaseDiff <= 1);
        }
    }

    if (expectedDoneCnt != promiseChecker.fDoneCount) {
        result = false;
        REPORTER_ASSERT(reporter, expectedDoneCnt == promiseChecker.fDoneCount);
    }

    return result;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTestNoDelayedRelease, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

    for (bool releaseImageEarly : {true, false}) {
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
                        SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo));

        SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
        SkCanvas* canvas = surface->getCanvas();

        int expectedFulfillCnt = 0;
        int expectedReleaseCnt = 0;
        int expectedDoneCnt = 0;
        ReleaseBalanceExpecation balanceExpecation = ReleaseBalanceExpecation::kBalanced;

        canvas->drawImage(refImg, 0, 0);
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        bool isVulkan = GrBackendApi::kVulkan == ctx->backend();
        canvas->flush();
        expectedFulfillCnt++;
        expectedReleaseCnt++;
        if (isVulkan) {
            balanceExpecation = ReleaseBalanceExpecation::kBalancedOrPlusOne;
        }
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 !isVulkan,
                                                                 expectedDoneCnt,
                                                                 reporter));

        gpu->testingOnly_flushGpuAndSync();
        balanceExpecation = ReleaseBalanceExpecation::kBalanced;
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        canvas->drawImage(refImg, 0, 0);
        canvas->drawImage(refImg, 0, 0);

        canvas->flush();
        expectedFulfillCnt++;
        expectedReleaseCnt++;

        gpu->testingOnly_flushGpuAndSync();
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        // Now test code path on Vulkan where we released the texture, but the GPU isn't done with
        // resource yet and we do another draw. We should only call fulfill on the first draw and
        // use the cached GrBackendTexture on the second. Release should only be called after the
        // second draw is finished.
        canvas->drawImage(refImg, 0, 0);
        canvas->flush();
        expectedFulfillCnt++;
        expectedReleaseCnt++;
        if (isVulkan) {
            balanceExpecation = ReleaseBalanceExpecation::kBalancedOrPlusOne;
        }
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 !isVulkan,
                                                                 expectedDoneCnt,
                                                                 reporter));

        canvas->drawImage(refImg, 0, 0);

        if (releaseImageEarly) {
            refImg.reset();
        }

        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 !isVulkan,
                                                                 expectedDoneCnt,
                                                                 reporter));

        canvas->flush();
        expectedFulfillCnt++;

        gpu->testingOnly_flushGpuAndSync();
        expectedReleaseCnt++;
        if (releaseImageEarly) {
            expectedDoneCnt++;
        }
        balanceExpecation = ReleaseBalanceExpecation::kBalanced;
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 !isVulkan,
                                                                 expectedDoneCnt,
                                                                 reporter));
        expectedFulfillCnt = promiseChecker.fFulfillCount;
        expectedReleaseCnt = promiseChecker.fReleaseCount;

        if (!releaseImageEarly) {
            refImg.reset();
            expectedDoneCnt++;
        }

        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        gpu->deleteTestingOnlyBackendTexture(backendTex);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTestDelayedRelease, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

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
                    SkDeferredDisplayListRecorder::DelayReleaseCallback::kYes));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    int expectedFulfillCnt = 0;
    int expectedReleaseCnt = 0;
    int expectedDoneCnt = 0;
    ReleaseBalanceExpecation balanceExpecation = ReleaseBalanceExpecation::kBalanced;

    canvas->drawImage(refImg, 0, 0);
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    bool isVulkan = GrBackendApi::kVulkan == ctx->backend();
    canvas->flush();
    expectedFulfillCnt++;
    // Because we've delayed release, we expect a +1 balance.
    balanceExpecation = ReleaseBalanceExpecation::kBalancedOrPlusOne;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));

    gpu->testingOnly_flushGpuAndSync();
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg, 0, 0);

    canvas->flush();

    gpu->testingOnly_flushGpuAndSync();
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    canvas->drawImage(refImg, 0, 0);
    canvas->flush();
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));

    canvas->drawImage(refImg, 0, 0);

    refImg.reset();

    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));

    canvas->flush();
    gpu->testingOnly_flushGpuAndSync();
    // We released the image already and we flushed and synced.
    balanceExpecation = ReleaseBalanceExpecation::kBalanced;
    expectedReleaseCnt++;
    expectedDoneCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));

    gpu->deleteTestingOnlyBackendTexture(backendTex);
}

// Tests replacing the backing texture for a promise image after a release and then refulfilling in
// the SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo case.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureReuse, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

    GrBackendTexture backendTex1 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, false, GrMipMapped::kNo);
    GrBackendTexture backendTex2 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, false, GrMipMapped::kNo);
    GrBackendTexture backendTex3 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, false, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex1.isValid());
    REPORTER_ASSERT(reporter, backendTex2.isValid());
    REPORTER_ASSERT(reporter, backendTex3.isValid());

    GrBackendFormat backendFormat = backendTex1.getBackendFormat();
    REPORTER_ASSERT(reporter, backendFormat.isValid());
    REPORTER_ASSERT(reporter, backendFormat == backendTex2.getBackendFormat());
    REPORTER_ASSERT(reporter, backendFormat == backendTex3.getBackendFormat());

    PromiseTextureChecker promiseChecker(backendTex1, reporter, true);
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
                    SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo));

    SkImageInfo info =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    int expectedFulfillCnt = 0;
    int expectedReleaseCnt = 0;
    int expectedDoneCnt = 0;

    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg, 5, 5);
    ReleaseBalanceExpecation balanceExpecation = ReleaseBalanceExpecation::kBalanced;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    bool isVulkan = GrBackendApi::kVulkan == ctx->backend();
    canvas->flush();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    if (isVulkan) {
        balanceExpecation = ReleaseBalanceExpecation::kBalancedOrPlusOne;
    }
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(
                                      promiseChecker.fLastFulfilledTexture, backendTex1));
    // We should have put a GrTexture for this fulfillment into the cache.
    auto keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 1);
    GrUniqueKey texKey1;
    if (keys.count()) {
        texKey1 = keys[0];
    }
    REPORTER_ASSERT(reporter, texKey1.isValid());
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey1));

    gpu->testingOnly_flushGpuAndSync();
    balanceExpecation = ReleaseBalanceExpecation::kBalanced;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));
    REPORTER_ASSERT(reporter,
                    GrBackendTexture::TestingOnly_Equals(
                            promiseChecker.replaceTexture()->backendTexture(), backendTex1));
    gpu->deleteTestingOnlyBackendTexture(backendTex1);

    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    // We should have invalidated the key on the previously cached texture (after ensuring
    // invalidation messages have been processed by calling purgeAsNeeded.)
    REPORTER_ASSERT(reporter, !ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey1));

    promiseChecker.replaceTexture(backendTex2);

    canvas->drawImage(refImg, 0, 0);
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    canvas->flush();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    if (isVulkan) {
        balanceExpecation = ReleaseBalanceExpecation::kBalancedOrPlusOne;
    }
    // Second texture should be in the cache.
    keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 1);
    GrUniqueKey texKey2;
    if (keys.count()) {
        texKey2 = keys[0];
    }
    REPORTER_ASSERT(reporter, texKey2.isValid() && texKey2 != texKey1);
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey2));

    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(
                                      promiseChecker.fLastFulfilledTexture, backendTex2));

    gpu->testingOnly_flushGpuAndSync();
    balanceExpecation = ReleaseBalanceExpecation::kBalanced;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    // Because we have kept the SkPromiseImageTexture alive, we should be able to use it again and
    // hit the cache.
    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey2));

    canvas->drawImage(refImg, 0, 0);

    canvas->flush();
    gpu->testingOnly_flushGpuAndSync();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    // Make sure we didn't add another key and that the second texture is still alive in the cache.
    keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 1);
    if (keys.count()) {
        REPORTER_ASSERT(reporter, texKey2 == keys[0]);
    }
    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey2));

    // Now we test keeping tex2 alive but fulfilling with a new texture.
    sk_sp<const SkPromiseImageTexture> promiseImageTexture2 =
            promiseChecker.replaceTexture(backendTex3);
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(
                                      promiseImageTexture2->backendTexture(), backendTex2));

    canvas->drawImage(refImg, 0, 0);

    canvas->flush();
    gpu->testingOnly_flushGpuAndSync();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 1);
    GrUniqueKey texKey3;
    if (keys.count()) {
        texKey3 = keys[0];
    }
    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    REPORTER_ASSERT(reporter, !ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey2));
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey3));
    gpu->deleteTestingOnlyBackendTexture(promiseImageTexture2->backendTexture());

    // Make a new promise image also backed by texture 3.
    sk_sp<SkImage> refImg2(
            SkImage_Gpu::MakePromiseTexture(
                    ctx, backendFormat, kWidth, kHeight,
                    GrMipMapped::kNo, texOrigin,
                    kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                    nullptr,
                    PromiseTextureChecker::Fulfill,
                    PromiseTextureChecker::Release,
                    PromiseTextureChecker::Done,
                    &promiseChecker,
                    SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo));
    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg2, 1, 1);

    canvas->flush();
    gpu->testingOnly_flushGpuAndSync();
    expectedFulfillCnt += 2;
    expectedReleaseCnt += 2;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    // The two images should share a single GrTexture by using the same key. The key is only
    // dependent on the pixel config and the PromiseImageTexture key.
    keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 1);
    if (keys.count() > 0) {
        REPORTER_ASSERT(reporter, texKey3 == keys[0]);
    }
    ctx->contextPriv().getResourceCache()->purgeAsNeeded();

    // If we delete the SkPromiseImageTexture we should trigger both key removals.
    REPORTER_ASSERT(reporter,
                    GrBackendTexture::TestingOnly_Equals(
                            promiseChecker.replaceTexture()->backendTexture(), backendTex3));

    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    REPORTER_ASSERT(reporter, !ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey3));
    gpu->deleteTestingOnlyBackendTexture(backendTex3);

    // After deleting each image we should get a done call.
    refImg.reset();
    ++expectedDoneCnt;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));
    refImg2.reset();
    ++expectedDoneCnt;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             balanceExpecation,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureReuseDifferentConfig, reporter, ctxInfo) {
    // Try making two promise SkImages backed by the same texture but with different configs.
    // This will only be testable on backends where a single texture format (8bit red unorm) can
    // be used for alpha and gray image color types.

    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

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

    for (auto delayRelease : {SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo,
                              SkDeferredDisplayListRecorder::DelayReleaseCallback::kYes}) {
        PromiseTextureChecker promiseChecker(backendTex1, reporter, true);
        sk_sp<SkImage> alphaImg(SkImage_Gpu::MakePromiseTexture(
                ctx, backendTex1.getBackendFormat(), kWidth, kHeight, GrMipMapped::kNo,
                kTopLeft_GrSurfaceOrigin, kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr,
                PromiseTextureChecker::Fulfill, PromiseTextureChecker::Release,
                PromiseTextureChecker::Done, &promiseChecker, delayRelease));
        REPORTER_ASSERT(reporter, alphaImg);

        sk_sp<SkImage> grayImg(SkImage_Gpu::MakePromiseTexture(
                ctx, backendTex1.getBackendFormat(), kWidth, kHeight, GrMipMapped::kNo,
                kBottomLeft_GrSurfaceOrigin, kGray_8_SkColorType, kOpaque_SkAlphaType, nullptr,
                PromiseTextureChecker::Fulfill, PromiseTextureChecker::Release,
                PromiseTextureChecker::Done, &promiseChecker, delayRelease));
        REPORTER_ASSERT(reporter, grayImg);

        canvas->drawImage(alphaImg, 0, 0);
        canvas->drawImage(grayImg, 1, 1);
        canvas->flush();
        gpu->testingOnly_flushGpuAndSync();

        int expectedFulfillCnt = 2;
        int expectedReleaseCnt = 0;
        int expectedDoneCnt = 0;
        ReleaseBalanceExpecation balanceExpecation = ReleaseBalanceExpecation::kAny;
        if (delayRelease == SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo) {
            expectedReleaseCnt = 2;
            balanceExpecation = ReleaseBalanceExpecation::kBalanced;
        }
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 balanceExpecation,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        // Because they use different configs, each image should have created a different GrTexture
        // and they both should still be cached.
        ctx->contextPriv().getResourceCache()->purgeAsNeeded();

        auto keys = promiseChecker.uniqueKeys();
        REPORTER_ASSERT(reporter, keys.count() == 2);
        for (const auto& key : keys) {
            auto surf = ctx->contextPriv().resourceProvider()->findByUniqueKey<GrSurface>(key);
            REPORTER_ASSERT(reporter, surf && surf->asTexture());
            if (surf && surf->asTexture()) {
                REPORTER_ASSERT(reporter,
                                !GrBackendTexture::TestingOnly_Equals(
                                        backendTex1, surf->asTexture()->getBackendTexture()));
            }
        }

        // Change the backing texture, this should invalidate the keys.
        promiseChecker.replaceTexture();
        ctx->contextPriv().getResourceCache()->purgeAsNeeded();

        for (const auto& key : keys) {
            auto surf = ctx->contextPriv().resourceProvider()->findByUniqueKey<GrSurface>(key);
            REPORTER_ASSERT(reporter, !surf);
        }
    }
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
            GrGpu* gpu = ctx->contextPriv().getGpu();

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
                    SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo));
            REPORTER_ASSERT(reporter, image);

            canvas->drawImage(image, 0, 0);
            image.reset();
            // If the surface still holds a ref to the context then the factory will not be able
            // to destroy the context (and instead will release-all-and-abandon).
            surface.reset();

            ctx->flush();
            contextDeath(&factory, ctx);

            int expectedFulfillCnt = 1;
            int expectedReleaseCnt = 1;
            int expectedDoneCnt = 1;
            ReleaseBalanceExpecation balanceExpecation = ReleaseBalanceExpecation::kBalanced;
            REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                     balanceExpecation,
                                                                     expectedFulfillCnt,
                                                                     expectedReleaseCnt,
                                                                     true,
                                                                     expectedDoneCnt,
                                                                     reporter));
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureFullCache, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

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
            SkDeferredDisplayListRecorder::DelayReleaseCallback::kNo));
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
        textures[i] = ctx->contextPriv().resourceProvider()->createTexture(desc, SkBudgeted::kYes);
        REPORTER_ASSERT(reporter, textures[i]);
    }

    // Relying on the asserts in the promiseImageChecker to ensure that fulfills and releases are
    // properly ordered.
    canvas->drawImage(image, 0, 0);
    canvas->flush();
    canvas->drawImage(image, 1, 0);
    canvas->flush();
    canvas->drawImage(image, 2, 0);
    canvas->flush();
    canvas->drawImage(image, 3, 0);
    canvas->flush();
    canvas->drawImage(image, 4, 0);
    canvas->flush();
    canvas->drawImage(image, 5, 0);
    canvas->flush();
    // Must call this to ensure that all callbacks are performed before the checker is destroyed.
    gpu->testingOnly_flushGpuAndSync();
    gpu->deleteTestingOnlyBackendTexture(backendTex);
}
