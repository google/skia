/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrTest.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkImage_Gpu.h"

using namespace sk_gpu_test;

struct PromiseTextureChecker {
    explicit PromiseTextureChecker(const GrBackendTexture& tex)
            : fTexture(tex)
            , fFulfillCount(0)
            , fReleaseCount(0) {}
    GrBackendTexture fTexture;
    int fFulfillCount;
    int fReleaseCount;
    static void Fulfill(void* self, GrBackendTexture* outTexture) {
        static_cast<PromiseTextureChecker*>(self)->fFulfillCount++;
        *outTexture = static_cast<PromiseTextureChecker*>(self)->fTexture;
    }
    static void Release(void* self) {
        static_cast<PromiseTextureChecker*>(self)->fReleaseCount++;
    }
};

static bool check_fulfill_and_release_cnts(const PromiseTextureChecker& promiseChecker,
                                           bool countsMustBeEqual,
                                           int expectedFulfillCnt,
                                           int expectedReleaseCnt,
                                           bool expectedRequired,
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

    int fulFillDiff = SkTAbs(expectedFulfillCnt - promiseChecker.fFulfillCount);
    if (fulFillDiff != 0) {
        if (expectedRequired) {
            result = false;
            REPORTER_ASSERT(reporter, expectedFulfillCnt == promiseChecker.fFulfillCount);
        } else if (fulFillDiff > 1) {
            REPORTER_ASSERT(reporter, fulFillDiff <= 1);
        }
    }

    int releaseDiff = SkTAbs(expectedReleaseCnt - promiseChecker.fReleaseCount);
    if (releaseDiff != 0) {
        if (expectedRequired) {
            result = false;
            REPORTER_ASSERT(reporter, expectedReleaseCnt == promiseChecker.fReleaseCount);
        } else if (releaseDiff > 1) {
            REPORTER_ASSERT(reporter, releaseDiff <= 1);
        }
    }

    return result;
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(PromiseImageTest, reporter, ctxInfo) {
    const int kWidth = 10;
    const int kHeight = 10;
    std::unique_ptr<uint32_t[]> pixels(new uint32_t[kWidth * kHeight]);

    GrContext* ctx = ctxInfo.grContext();
    GrGpu* gpu = ctx->contextPriv().getGpu();

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
               pixels.get(), kWidth, kHeight, kRGBA_8888_GrPixelConfig, true, GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrBackendFormat backendFormat = GrTest::CreateBackendFormatFromTexture(backendTex);
    REPORTER_ASSERT(reporter, backendFormat.isValid());

    PromiseTextureChecker promiseChecker(backendTex);
    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(
        SkImage_Gpu::MakePromiseTexture(ctx, backendFormat, kWidth, kHeight, GrMipMapped::kNo,
                                        texOrigin, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                        nullptr,
                                        PromiseTextureChecker::Fulfill,
                                        PromiseTextureChecker::Release,
                                        &promiseChecker));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();

    int expectedFulfillCnt = 0;
    int expectedReleaseCnt = 0;

    canvas->drawImage(refImg, 0, 0);
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             reporter));

    bool isVulkan = kVulkan_GrBackend == ctx->contextPriv().getBackend();
    canvas->flush();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             !isVulkan,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             reporter));

    gpu->testingOnly_flushGpuAndSync();
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
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
                                                             reporter));

    // Now test code path on Vulkan where we released the texture, but the GPU isn't done with
    // resource yet and we do another draw. We should only call fulfill on the first draw and
    // use the cached GrBackendTexture on the second. Release should only be called after the second
    // draw is finished.
    canvas->drawImage(refImg, 0, 0);
    canvas->flush();
    expectedFulfillCnt++;
    expectedReleaseCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             !isVulkan,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             reporter));

    canvas->drawImage(refImg, 0, 0);
    canvas->flush();
    expectedFulfillCnt++;

    gpu->testingOnly_flushGpuAndSync();
    expectedReleaseCnt++;
    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             !isVulkan,
                                                             reporter));
    expectedFulfillCnt = promiseChecker.fFulfillCount;
    expectedReleaseCnt = promiseChecker.fReleaseCount;

    refImg.reset();

    REPORTER_ASSERT(reporter, check_fulfill_and_release_cnts(promiseChecker,
                                                             true,
                                                             expectedFulfillCnt,
                                                             expectedReleaseCnt,
                                                             true,
                                                             reporter));

    gpu->deleteTestingOnlyBackendTexture(&backendTex);
}

#endif
