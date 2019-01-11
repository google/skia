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
#include "SkImage_Gpu.h"
#include "SkPromiseImageTexture.h"

using namespace sk_gpu_test;

struct PromiseTextureChecker {
    explicit PromiseTextureChecker(const GrBackendTexture& tex)
            : fTexture(SkPromiseImageTexture::Make(tex))
            , fFulfillCount(0)
            , fReleaseCount(0)
            , fDoneCount(0) {}
    sk_sp<SkPromiseImageTexture> fTexture;
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
        // Can't change this while in active fulfillment.
        SkASSERT(fFulfillCount == fReleaseCount);
        auto temp = std::move(fTexture);
        fTexture = SkPromiseImageTexture::Make(tex);
        return std::move(temp);
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
    static void Release(void* self) { static_cast<PromiseTextureChecker*>(self)->fReleaseCount++; }
    static void Done(void* self) {
        static_cast<PromiseTextureChecker*>(self)->fDoneCount++;
    }
};

// Because Vulkan may delay when it actually calls the ReleaseProcs depending on when command
// buffers finish their work, we need some slight wiggle room in what values we expect for fulfill
// and release counts.
static bool check_fulfill_and_release_cnts(const PromiseTextureChecker& promiseChecker,
                                           bool countsMustBeEqual,
                                           int expectedFulfillCnt,
                                           int expectedReleaseCnt,
                                           bool expectedRequired,
                                           int expectedDoneCnt,
                                           skiatest::Reporter* reporter) {
    bool result = true;
    int countDiff = promiseChecker.fFulfillCount - promiseChecker.fReleaseCount;
    // FulfillCount should always equal ReleaseCount or be at most one higher
    if (countDiff != 0) {
        if (countsMustBeEqual) {
            result = false;
            REPORTER_ASSERT(reporter, 0 == countDiff);
        } else if (countDiff != 1) {
            result = false;
            REPORTER_ASSERT(reporter, 0 == countDiff || 1 == countDiff);
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTest, reporter, ctxInfo) {
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

        PromiseTextureChecker promiseChecker(backendTex);
        GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
        sk_sp<SkImage> refImg(
                SkImage_Gpu::MakePromiseTexture(ctx, backendFormat, kWidth, kHeight,
                                                GrMipMapped::kNo, texOrigin,
                                                kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                                nullptr,
                                                PromiseTextureChecker::Fulfill,
                                                PromiseTextureChecker::Release,
                                                PromiseTextureChecker::Done,
                                                &promiseChecker));

        SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
        SkCanvas* canvas = surface->getCanvas();

        int expectedFulfillCnt = 0;
        int expectedReleaseCnt = 0;
        int expectedDoneCnt = 0;

        canvas->drawImage(refImg, 0, 0);
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 true,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        bool isVulkan = GrBackendApi::kVulkan == ctx->contextPriv().getBackend();
        canvas->flush();
        expectedFulfillCnt++;
        expectedReleaseCnt++;
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 !isVulkan,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 !isVulkan,
                                                                 expectedDoneCnt,
                                                                 reporter));

        gpu->testingOnly_flushGpuAndSync();
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 true,
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
                                                                 true,
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
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 !isVulkan,
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
                                                                 !isVulkan,
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
        REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                                 true,
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
                                                                 true,
                                                                 expectedFulfillCnt,
                                                                 expectedReleaseCnt,
                                                                 true,
                                                                 expectedDoneCnt,
                                                                 reporter));

        gpu->deleteTestingOnlyBackendTexture(backendTex);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureReuse, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

    GrBackendTexture backendTex1 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, true, GrMipMapped::kNo);
    GrBackendTexture backendTex2 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, true, GrMipMapped::kNo);
    GrBackendTexture backendTex3 = gpu->createTestingOnlyBackendTexture(
            nullptr, kWidth, kHeight, GrColorType::kRGBA_8888, true, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex1.isValid());
    REPORTER_ASSERT(reporter, backendTex2.isValid());
    REPORTER_ASSERT(reporter, backendTex3.isValid());

    GrBackendFormat backendFormat = backendTex1.getBackendFormat();
    REPORTER_ASSERT(reporter, backendFormat.isValid());
    REPORTER_ASSERT(reporter, backendFormat == backendTex2.getBackendFormat());
    REPORTER_ASSERT(reporter, backendFormat == backendTex3.getBackendFormat());

    PromiseTextureChecker promiseChecker(backendTex1);
    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(
            SkImage_Gpu::MakePromiseTexture(ctx, backendFormat, kWidth, kHeight,
                                            GrMipMapped::kNo, texOrigin,
                                            kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                            nullptr,
                                            PromiseTextureChecker::Fulfill,
                                            PromiseTextureChecker::Release,
                                            PromiseTextureChecker::Done,
                                            &promiseChecker));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    int expectedFulfillCnt = 0;
    int expectedReleaseCnt = 0;
    int expectedDoneCnt = 0;

    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg, 5, 5);
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    bool isVulkan = GrBackendApi::kVulkan == ctx->contextPriv().getBackend();
    canvas->flush();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             !isVulkan,
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
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
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
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    canvas->flush();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
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
                                                             !isVulkan,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             expectedDoneCnt,
                                                             reporter));
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(
                                      promiseChecker.fLastFulfilledTexture, backendTex2));

    gpu->testingOnly_flushGpuAndSync();
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
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
                                                             true,
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
                                                             true,
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
            SkImage_Gpu::MakePromiseTexture(ctx, backendFormat, kWidth, kHeight,
                                            GrMipMapped::kNo, texOrigin,
                                            kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                            nullptr,
                                            PromiseTextureChecker::Fulfill,
                                            PromiseTextureChecker::Release,
                                            PromiseTextureChecker::Done,
                                            &promiseChecker));
    canvas->drawImage(refImg, 0, 0);
    canvas->drawImage(refImg2, 1, 1);

    canvas->flush();
    gpu->testingOnly_flushGpuAndSync();
    expectedFulfillCnt += 2;
    expectedReleaseCnt += 2;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));

    // We currently expect each promise image to make and cache its own GrTexture. We will likely
    // try to make these share in the future.
    keys = promiseChecker.uniqueKeys();
    REPORTER_ASSERT(reporter, keys.count() == 2);
    GrUniqueKey texKey4;
    if (keys.count() == 2) {
        REPORTER_ASSERT(reporter, texKey3 == keys[0]);
        texKey4 = keys[1];
    }
    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey3));
    REPORTER_ASSERT(reporter, ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey4));

    // If we delete the SkPromiseImageTexture we should trigger both key removals.
    REPORTER_ASSERT(reporter,
                    GrBackendTexture::TestingOnly_Equals(
                            promiseChecker.replaceTexture()->backendTexture(), backendTex3));

    ctx->contextPriv().getResourceCache()->purgeAsNeeded();
    REPORTER_ASSERT(reporter, !ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey3));
    REPORTER_ASSERT(reporter, !ctx->contextPriv().resourceProvider()->findByUniqueKey<>(texKey4));
    gpu->deleteTestingOnlyBackendTexture(backendTex3);

    // After deleting each image we should get a done call.
    refImg.reset();
    ++expectedDoneCnt;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));
    refImg2.reset();
    ++expectedDoneCnt;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             expectedDoneCnt,
                                                             reporter));
}
