/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/YUVABackendTextures.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "tests/Test.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;

namespace {

struct PromiseImageChecker {
    PromiseImageChecker() = default;

    void checkImageReleased(skiatest::Reporter* reporter, int expectedReleaseCnt) {
        REPORTER_ASSERT(reporter, expectedReleaseCnt == fImageReleaseCount);
    }

    int fImageReleaseCount = 0;

    static void ImageRelease(void* self) {
        auto checker = reinterpret_cast<PromiseImageChecker*>(self);

        checker->fImageReleaseCount++;
    }
};

struct PromiseTextureChecker {
    PromiseTextureChecker() = default;

    explicit PromiseTextureChecker(const BackendTexture& backendTex) {
        fBackendTextures[0] = backendTex;
    }

    explicit PromiseTextureChecker(const BackendTexture& backendTex0,
                                   const BackendTexture& backendTex1)
            : fHasTwoBackendTextures(true) {
        fBackendTextures[0] = backendTex0;
        fBackendTextures[1] = backendTex1;
    }

    int totalReleaseCount() const { return fTextureReleaseCounts[0] + fTextureReleaseCounts[1]; }

    bool fHasTwoBackendTextures = false;
    BackendTexture fBackendTextures[2];
    int fFulfillCount = 0;
    int fTextureReleaseCounts[2] = { 0, 0 };

    static std::tuple<BackendTexture, void*> Fulfill(void* self) {
        auto checker = reinterpret_cast<PromiseTextureChecker*>(self);

        checker->fFulfillCount++;

        if (checker->fHasTwoBackendTextures) {
            int whichToUse = checker->fFulfillCount % 2;
            return { checker->fBackendTextures[whichToUse],
                     &checker->fTextureReleaseCounts[whichToUse] };
        } else {
            return { checker->fBackendTextures[0], &checker->fTextureReleaseCounts[0] };
        }
    }

    static void TextureRelease(void* context) {
        int* releaseCount = reinterpret_cast<int*>(context);

        (*releaseCount)++;
    }
};

enum class ReleaseBalanceExpectation {
    kBalanced,
    kOffByOne,     // fulfill calls ahead of release calls by 1
    kOffByTwo,     // fulfill calls ahead of release calls by 2
    kFulfillsOnly, // 'n' fulfill calls, 0 release calls
};

void check_fulfill_and_release_cnts(skiatest::Reporter* reporter,
                                    const PromiseImageChecker& promiseImageChecker,
                                    const PromiseTextureChecker* promiseTextureCheckers,
                                    int expectedFulfillCnt,
                                    ReleaseBalanceExpectation releaseBalanceExpectation) {
    // If one fulfill fails the following ones won't be called, so we check the max value
    int maxFulfillCnt = 0;
    for (int i = 0; i < 4; ++i) {
        maxFulfillCnt = std::max(promiseTextureCheckers[i].fFulfillCount, maxFulfillCnt);
        if (!expectedFulfillCnt) {
            // Release should only ever be called after Fulfill.
            REPORTER_ASSERT(reporter, promiseImageChecker.fImageReleaseCount == 0);
            REPORTER_ASSERT(reporter, promiseTextureCheckers[i].totalReleaseCount() == 0);
            return;
        }

        if (promiseTextureCheckers[i].fFulfillCount) {
            int releaseDiff = promiseTextureCheckers[i].fFulfillCount -
                              promiseTextureCheckers[i].totalReleaseCount();
            switch (releaseBalanceExpectation) {
                case ReleaseBalanceExpectation::kBalanced:
                    SkASSERT(!releaseDiff);
                    REPORTER_ASSERT(reporter, !releaseDiff);
                    break;
                case ReleaseBalanceExpectation::kOffByOne:
                    SkASSERT(releaseDiff == 1);
                    REPORTER_ASSERT(reporter, releaseDiff == 1);
                    break;
                case ReleaseBalanceExpectation::kOffByTwo:
                    SkASSERT(releaseDiff == 2);
                    REPORTER_ASSERT(reporter, releaseDiff == 2);
                    break;
                case ReleaseBalanceExpectation::kFulfillsOnly:
                    REPORTER_ASSERT(reporter, promiseTextureCheckers[i].totalReleaseCount() == 0);
                    break;
            }
        }
    }
    SkASSERT(maxFulfillCnt == expectedFulfillCnt);
    REPORTER_ASSERT(reporter, maxFulfillCnt == expectedFulfillCnt);
}

void check_unfulfilled(const PromiseImageChecker& promiseImageChecker,
                       const PromiseTextureChecker* promiseTextureCheckers,
                       skiatest::Reporter* reporter) {
    check_fulfill_and_release_cnts(reporter, promiseImageChecker, promiseTextureCheckers,
                                   /* expectedFulfillCnt= */ 0,
                                   ReleaseBalanceExpectation::kBalanced);
}

void check_fulfilled_ahead_by_one(skiatest::Reporter* reporter,
                                  const PromiseImageChecker& promiseImageChecker,
                                  const PromiseTextureChecker* promiseTextureCheckers,
                                  int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseImageChecker, promiseTextureCheckers,
                                   expectedFulfillCnt, ReleaseBalanceExpectation::kOffByOne);
}

void check_fulfilled_ahead_by_two(skiatest::Reporter* reporter,
                                  const PromiseImageChecker& promiseImageChecker,
                                  const PromiseTextureChecker* promiseTextureCheckers,
                                  int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseImageChecker, promiseTextureCheckers,
                                   expectedFulfillCnt, ReleaseBalanceExpectation::kOffByTwo);
}

void check_all_done(skiatest::Reporter* reporter,
                    const PromiseImageChecker& promiseImageChecker,
                    const PromiseTextureChecker* promiseTextureCheckers,
                    int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseImageChecker, promiseTextureCheckers,
                                   expectedFulfillCnt, ReleaseBalanceExpectation::kBalanced);
}

void check_fulfills_only(skiatest::Reporter* reporter,
                         const PromiseImageChecker& promiseImageChecker,
                         const PromiseTextureChecker* promiseTextureCheckers,
                         int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseImageChecker, promiseTextureCheckers,
                                   expectedFulfillCnt, ReleaseBalanceExpectation::kFulfillsOnly);
}

struct TestCtx {
    TestCtx() {}

    ~TestCtx() {
        for (int i = 0; i < 8; ++i) {
            if (fBackendTextures[i].isValid()) {
                fContext->deleteBackendTexture(fBackendTextures[i]);
            }
        }
    }

    Context* fContext;
    std::unique_ptr<Recorder> fRecorder;
    BackendTexture fBackendTextures[8];
    PromiseImageChecker fPromiseImageChecker;
    SkImages::GraphitePromiseImageContext fImageContext = &fPromiseImageChecker;
    PromiseTextureChecker fPromiseTextureCheckers[4];
    SkImages::GraphitePromiseTextureContext fTextureContexts[4] = {
        &fPromiseTextureCheckers[0],
        &fPromiseTextureCheckers[1],
        &fPromiseTextureCheckers[2],
        &fPromiseTextureCheckers[3],
    };
    sk_sp<SkImage> fImg;
    sk_sp<SkSurface> fSurface;
};

void setup_test_context(Context* context,
                        skiatest::Reporter* reporter,
                        TestCtx* testCtx,
                        SkISize dimensions,
                        Volatile isVolatile,
                        bool invalidBackendTex) {
    testCtx->fContext = context;

    const Caps* caps = context->priv().caps();

    skgpu::Protected isProtected = skgpu::Protected(caps->protectedSupport());

    testCtx->fRecorder = context->makeRecorder();

    testCtx->fPromiseImageChecker = PromiseImageChecker();

    TextureInfo textureInfo[4];
    for (int i = 0; i < 4; ++i) {
        textureInfo[i] = caps->getDefaultSampledTextureInfo(kAlpha_8_SkColorType,
                                                            skgpu::Mipmapped::kNo,
                                                            isProtected,
                                                            skgpu::Renderable::kYes);

        if (invalidBackendTex) {
            // Having invalid backend textures will invalidate all the fulfill calls
            REPORTER_ASSERT(reporter, !testCtx->fBackendTextures[i].isValid());
            REPORTER_ASSERT(reporter, !testCtx->fBackendTextures[i+4].isValid());
        } else {
            testCtx->fBackendTextures[i] =
                    testCtx->fRecorder->createBackendTexture(dimensions, textureInfo[i]);
            REPORTER_ASSERT(reporter, testCtx->fBackendTextures[i].isValid());

            if (isVolatile == Volatile::kYes) {
                testCtx->fBackendTextures[i+4] =
                        testCtx->fRecorder->createBackendTexture(dimensions, textureInfo[i]);
                REPORTER_ASSERT(reporter, testCtx->fBackendTextures[i+4].isValid());
            }
        }

        if (isVolatile == Volatile::kYes) {
            testCtx->fPromiseTextureCheckers[i] =
                    PromiseTextureChecker(testCtx->fBackendTextures[i],
                                          testCtx->fBackendTextures[i+4]);
        } else {
            testCtx->fPromiseTextureCheckers[i] =
                     PromiseTextureChecker(testCtx->fBackendTextures[i]);
        }
    }

    SkYUVAInfo yuvaInfo(dimensions,
                        SkYUVAInfo::PlaneConfig::kY_U_V_A,
                        SkYUVAInfo::Subsampling::k444,
                        kJPEG_Full_SkYUVColorSpace);
    YUVABackendTextureInfo yuvaBackendInfo(testCtx->fRecorder.get(),
                                           yuvaInfo,
                                           textureInfo,
                                           skgpu::Mipmapped::kNo);

    testCtx->fImg = SkImages::PromiseTextureFromYUVA(testCtx->fRecorder.get(),
                                                     yuvaBackendInfo,
                                                     SkColorSpace::MakeSRGBLinear(),
                                                     isVolatile,
                                                     PromiseTextureChecker::Fulfill,
                                                     PromiseImageChecker::ImageRelease,
                                                     PromiseTextureChecker::TextureRelease,
                                                     testCtx->fImageContext,
                                                     testCtx->fTextureContexts);

    SkImageInfo ii = SkImageInfo::Make(dimensions.fWidth,
                                       dimensions.fHeight,
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    testCtx->fSurface = SkSurfaces::RenderTarget(testCtx->fRecorder.get(), ii);
}

} // anonymous namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(NonVolatileGraphiteYUVAPromiseImageTest,
                                                     reporter,
                                                     context,
                                                     testGpuContext,
                                                     true,
                                                     CtsEnforcement::kNextRelease) {
    constexpr SkISize kDimensions { 16, 16 };

    TestCtx testContext;
    setup_test_context(context, reporter, &testContext,
                       kDimensions, Volatile::kNo, /* invalidBackendTex= */ false);

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter); // NVPIs not fulfilled at snap

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1); // NVPIs fulfilled at insert
    }

    context->submit(SyncToCpu::kNo);
    // testContext.fImg still has a ref so we should not have called TextureRelease.
    check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                 testContext.fPromiseTextureCheckers,
                                 /* expectedFulfillCnt= */ 1);

    testGpuContext->syncedSubmit(context);

    check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                 testContext.fPromiseTextureCheckers,
                                 /* expectedFulfillCnt= */ 1);

    // Test that more draws and insertions don't refulfill the NVPI
    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        canvas->drawImage(testContext.fImg, 0, 0);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1); // No new fulfill

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        // testContext.fImg should still be fulfilled from the first time we inserted a Recording.
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1);
    }

    testGpuContext->syncedSubmit(context);

    check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                 testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 1);

    // Test that dropping the SkImage's ref doesn't change anything
    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        testContext.fImg.reset();

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1);

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1);
    }

    // fImg's proxy is reffed by the recording so, despite fImg being reset earlier,
    // the imageRelease callback doesn't occur until the recording is deleted.
    testContext.fPromiseImageChecker.checkImageReleased(reporter, /* expectedReleaseCnt= */ 1);

    // testContext.fImg no longer holds a ref but the last recording is still not submitted.
    check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                 testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 1);

    testGpuContext->syncedSubmit(context);

    // Now TextureRelease should definitely have been called.
    check_all_done(reporter, testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                   /* expectedFulfillCnt= */ 1);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(
        NonVolatileGraphiteYUVAPromiseImageFulfillFailureTest,
        reporter,
        context,
        testGpuContext,
        true,
        CtsEnforcement::kNextRelease) {
    constexpr SkISize kDimensions { 16, 16 };

    TestCtx testContext;
    setup_test_context(context, reporter, &testContext,
                       kDimensions, Volatile::kNo, /* invalidBackendTex= */ true);

    // Draw the image a few different ways.
    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1);

        // Test that reinserting gives uninstantiated PromiseImages a second chance
        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 2);
    }

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        SkPaint paint;
        paint.setColorFilter(SkColorFilters::LinearToSRGBGamma());
        canvas->drawImage(testContext.fImg, 0, 0, SkSamplingOptions(), &paint);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 2);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 3);
    }

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        sk_sp<SkShader> shader = testContext.fImg->makeShader(SkSamplingOptions());
        REPORTER_ASSERT(reporter, shader);

        SkPaint paint;
        paint.setShader(std::move(shader));
        canvas->drawRect(SkRect::MakeWH(1, 1), paint);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 3);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 4);
    }

    testContext.fSurface.reset();
    testContext.fImg.reset();

    // Despite fulfill failing 4x, the imageRelease callback still fires
    testContext.fPromiseImageChecker.checkImageReleased(reporter, /* expectedReleaseCnt= */ 1);

    testGpuContext->syncedSubmit(context);
    // fulfill should've been called 4x while release should never have been called
    check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                        testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 4);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(NonVolatileGraphiteYUVAPromiseImageCreationFailureTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // Note: these dimensions are invalid and will cause MakeGraphitePromiseTexture to fail
    constexpr SkISize kDimensions { 0, 0 };

    TestCtx testContext;
    setup_test_context(context, reporter, &testContext,
                       kDimensions, Volatile::kNo, /* invalidBackendTex= */ true);

    SkASSERT(!testContext.fImg);

    // Despite MakeGraphitePromiseTexture failing, ImageRelease is called
    REPORTER_ASSERT(reporter, testContext.fPromiseImageChecker.fImageReleaseCount == 1);
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, testContext.fPromiseTextureCheckers[i].fFulfillCount == 0);
        REPORTER_ASSERT(reporter, testContext.fPromiseTextureCheckers[i].totalReleaseCount() == 0);
    }
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(VolatileGraphiteYUVAPromiseImageTest,
                                                     reporter,
                                                     context,
                                                     testGpuContext,
                                                     true,
                                                     CtsEnforcement::kNextRelease) {
    constexpr SkISize kDimensions { 16, 16 };

    TestCtx testContext;
    setup_test_context(context, reporter, &testContext,
                       kDimensions, Volatile::kYes, /* invalidBackendTex= */ false);

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        // Nothing happens at snap time for VPIs
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 1);  // VPIs fulfilled on insert

        // Test that multiple insertions will clobber prior fulfills
        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_two(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 2);
    }

    testGpuContext->syncedSubmit(context);
    check_all_done(reporter, testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                   /* expectedFulfillCnt= */ 2);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                        testContext.fPromiseTextureCheckers[i].fTextureReleaseCounts[0] == 1);
        REPORTER_ASSERT(reporter,
                        testContext.fPromiseTextureCheckers[i].fTextureReleaseCounts[1] == 1);
    }

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        canvas->drawImage(testContext.fImg, 0, 0);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        // Nothing happens at snap time for volatile images
        check_all_done(reporter, testContext.fPromiseImageChecker,
                       testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 2);

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 3);

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_two(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 4);
    }

    testGpuContext->syncedSubmit(context);
    check_all_done(reporter, testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                   /* expectedFulfillCnt= */ 4);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                        testContext.fPromiseTextureCheckers[i].fTextureReleaseCounts[0] == 2);
        REPORTER_ASSERT(reporter,
                        testContext.fPromiseTextureCheckers[i].fTextureReleaseCounts[1] == 2);
    }

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        testContext.fImg.reset();

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        // Nothing happens at snap time for volatile images
        check_all_done(reporter, testContext.fPromiseImageChecker,
                       testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 4);

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_one(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 5);

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfilled_ahead_by_two(reporter, testContext.fPromiseImageChecker,
                                     testContext.fPromiseTextureCheckers,
                                     /* expectedFulfillCnt= */ 6);
    }

    // testContext.fImg no longer holds a ref but the last recordings are still not submitted.
    check_fulfilled_ahead_by_two(reporter, testContext.fPromiseImageChecker,
                                 testContext.fPromiseTextureCheckers,
                                 /* expectedFulfillCnt= */ 6);

    testGpuContext->syncedSubmit(context);

    // Now all Releases should definitely have been called.
    check_all_done(reporter, testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                   /* expectedFulfillCnt= */ 6);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                        testContext.fPromiseTextureCheckers[i].fTextureReleaseCounts[0] == 3);
        REPORTER_ASSERT(reporter,
                        testContext.fPromiseTextureCheckers[i].fTextureReleaseCounts[1] == 3);
    }
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(
        VolatileGraphiteYUVAPromiseImageFulfillFailureTest,
        reporter,
        context,
        testGpuContext,
        true,
        CtsEnforcement::kNextRelease) {
    constexpr SkISize kDimensions { 16, 16 };

    TestCtx testContext;
    setup_test_context(context, reporter, &testContext,
                       kDimensions, Volatile::kYes, /* invalidBackendTex= */ true);

    // Draw the image a few different ways.
    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 1);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 2);
    }

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        SkPaint paint;
        paint.setColorFilter(SkColorFilters::LinearToSRGBGamma());
        canvas->drawImage(testContext.fImg, 0, 0, SkSamplingOptions(), &paint);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 2);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 3);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 4);
    }

    {
        SkCanvas* canvas = testContext.fSurface->getCanvas();

        sk_sp<SkShader> shader = testContext.fImg->makeShader(SkSamplingOptions());
        REPORTER_ASSERT(reporter, shader);

        SkPaint paint;
        paint.setShader(std::move(shader));
        canvas->drawRect(SkRect::MakeWH(1, 1), paint);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 4);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 5);

        REPORTER_ASSERT(reporter, !context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 6);
    }

    testContext.fSurface.reset();
    testContext.fImg.reset();

    testGpuContext->syncedSubmit(context);
    check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                        testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 6);
}

// Test out dropping the Recorder prior to inserting the Recording
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(GraphiteYUVAPromiseImageRecorderLoss,
                                                     reporter,
                                                     context,
                                                     testGpuContext,
                                                     true,
                                                     CtsEnforcement::kNextRelease) {
    constexpr SkISize kDimensions{ 16, 16 };

    for (Volatile isVolatile : { Volatile::kNo, Volatile::kYes }) {
        TestCtx testContext;
        setup_test_context(context, reporter, &testContext,
                           kDimensions, isVolatile, /* invalidBackendTex= */ false);

        SkCanvas* canvas = testContext.fSurface->getCanvas();

        canvas->drawImage(testContext.fImg, 0, 0);
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        std::unique_ptr<Recording> recording = testContext.fRecorder->snap();
        check_unfulfilled(testContext.fPromiseImageChecker, testContext.fPromiseTextureCheckers,
                          reporter);

        testContext.fRecorder.reset();  // Recorder drop

        REPORTER_ASSERT(reporter, context->insertRecording({ recording.get() }));
        check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                            testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 1);

        testGpuContext->syncedSubmit(context);

        testContext.fSurface.reset();
        testContext.fImg.reset();
        recording.reset();

        check_all_done(reporter, testContext.fPromiseImageChecker,
                       testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 1);
    }
}

// Test out PromiseImages appearing in multiple Recordings. In particular, test that
// previous instantiations don't impact the Recording's collection of PromiseImages.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(GraphiteYUVAPromiseImageMultipleImgUses,
                                                     reporter,
                                                     context,
                                                     testGpuContext,
                                                     true,
                                                     CtsEnforcement::kNextRelease) {
    constexpr SkISize kDimensions{ 16, 16 };

    static constexpr int kNumRecordings = 3;

    for (Volatile isVolatile : { Volatile::kNo, Volatile::kYes }) {
        int expectedVolatile = (isVolatile == Volatile::kYes) ? 4 : 0;
        int expectedNonVolatile = 4 - expectedVolatile;

        TestCtx testContext;
        setup_test_context(context, reporter, &testContext,
                           kDimensions, isVolatile, /* invalidBackendTex= */ false);

        std::unique_ptr<Recording> recordings[kNumRecordings];

        SkCanvas* canvas = testContext.fSurface->getCanvas();

        for (int i = 0; i < kNumRecordings; ++i) {
            canvas->drawImage(testContext.fImg, 0, 0);

            recordings[i] = testContext.fRecorder->snap();

            if (isVolatile == Volatile::kYes) {
                check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                                    testContext.fPromiseTextureCheckers,
                                    /* expectedFulfillCnt= */ i);
            } else {
                check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                                    testContext.fPromiseTextureCheckers,
                                    /* expectedFulfillCnt= */ i > 0 ? 1 : 0);
            }

            REPORTER_ASSERT(reporter,
                            recordings[i]->priv().numVolatilePromiseImages() == expectedVolatile);
            REPORTER_ASSERT(reporter,
                            recordings[i]->priv().numNonVolatilePromiseImages() ==
                            expectedNonVolatile);

            REPORTER_ASSERT(reporter, context->insertRecording({ recordings[i].get() }));

            if (isVolatile == Volatile::kYes) {
                check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                                    testContext.fPromiseTextureCheckers,
                                    /* expectedFulfillCnt= */ i+1);
            } else {
                check_fulfills_only(reporter, testContext.fPromiseImageChecker,
                                    testContext.fPromiseTextureCheckers,
                                    /* expectedFulfillCnt= */ 1);
            }

            // Non-volatiles are cleared out after a successful insertion
            REPORTER_ASSERT(reporter, recordings[i]->priv().numNonVolatilePromiseImages() == 0);
        }

        testGpuContext->syncedSubmit(context);

        testContext.fSurface.reset();
        testContext.fImg.reset();
        for (int i = 0; i < kNumRecordings; ++i) {
            recordings[i].reset();
        }

        if (isVolatile == Volatile::kYes) {
            check_all_done(reporter, testContext.fPromiseImageChecker,
                           testContext.fPromiseTextureCheckers,
                           /* expectedFulfillCnt= */ kNumRecordings);
        } else {
            check_all_done(reporter, testContext.fPromiseImageChecker,
                           testContext.fPromiseTextureCheckers, /* expectedFulfillCnt= */ 1);
        }
    }
}
