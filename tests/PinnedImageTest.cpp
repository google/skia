/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.
#include "include/android/SkImageAndroid.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h" // IWYU pragma: keep
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/FenceSync.h"

#include <string>

class GrRecordingContext;
struct GrContextOptions;

using namespace sk_gpu_test;

static bool surface_is_expected_color(SkSurface* surf, const SkImageInfo& ii, SkColor color) {
    SkBitmap bm;
    bm.allocPixels(ii);

    surf->readPixels(bm, 0, 0);

    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (bm.getColor(x, y) != color) {
                return false;
            }
        }
    }

    return true;
}

static void basic_test(skiatest::Reporter* reporter, GrRecordingContext* rContext) {
    skiatest::ReporterContext subtest(reporter, "basic_test");
    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorRED);

    // We start off with the raster image being all red.
    sk_sp<SkImage> img = SkImages::PinnableRasterFromBitmap(bm);
    REPORTER_ASSERT(reporter, img, "PinnableImageFromBitmap returned null");

    sk_sp<SkSurface> gpuSurface = SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kYes, ii);
    SkCanvas* canvas = gpuSurface->getCanvas();

    // w/o pinning - the gpu draw always reflects the current state of the underlying bitmap
    {
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorRED));

        bmCanvas.clear(SK_ColorGREEN);

        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));
    }

    // w/ pinning - the gpu draw is stuck at the pinned state
    {
        bool ok = skgpu::ganesh::PinAsTexture(rContext, img.get()); // pin at blue
        REPORTER_ASSERT(reporter, ok, "PinAsTexture did not succeed");

        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));

        bmCanvas.clear(SK_ColorBLUE);

        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));

        skgpu::ganesh::UnpinTexture(rContext, img.get());
    }

    // once unpinned local changes will be picked up
    {
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorBLUE));
    }
}

// Deleting the context while there are still pinned images shouldn't result in a crash.
static void cleanup_test(skiatest::Reporter* reporter) {
    skiatest::ReporterContext subtest(reporter, "cleanup_test");
    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorRED);

    GrMockOptions options;
    sk_sp<GrDirectContext> mockContext = GrDirectContext::MakeMock(&options);

    for (int i = 0; i < skgpu::kContextTypeCount; ++i) {
        auto ctxType = static_cast<skgpu::ContextType>(i);

        {
            sk_sp<SkImage> img;
            GrDirectContext* dContext = nullptr;

            {
                GrContextFactory testFactory;
                ContextInfo info = testFactory.getContextInfo(ctxType);
                dContext = info.directContext();
                if (!dContext) {
                    continue;
                }

                img = SkImages::PinnableRasterFromBitmap(bm);
                if (!skgpu::ganesh::PinAsTexture(dContext, img.get())) {
                    continue;
                }
                // Pinning on a second context should be blocked.
                REPORTER_ASSERT(reporter, !skgpu::ganesh::PinAsTexture(mockContext.get(),
                                                                       img.get()));
            }

            // The context used to pin the image is gone at this point!
            // "context" isn't technically used in this call but it can't be null!
            // We don't really want to support this use case but it currently happens.
            skgpu::ganesh::UnpinTexture(dContext, img.get());
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PinnedImageTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {

    basic_test(reporter, ctxInfo.directContext());
    cleanup_test(reporter);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PinnedImageTest_AsGaneshView,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    GrRecordingContext* rContext = ctxInfo.directContext();
    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorMAGENTA); // arbitrary color

    sk_sp<SkImage> img = SkImages::PinnableRasterFromBitmap(bm);
    REPORTER_ASSERT(reporter, img, "PinnableImageFromBitmap returned null");

    {
        skiatest::ReporterContext subtest(reporter, "cached path");
        auto [view, colortype] = skgpu::ganesh::AsView(rContext, img, skgpu::Mipmapped::kNo,
                   GrImageTexGenPolicy::kDraw);
        REPORTER_ASSERT(reporter, view, "AsView returned falsey view");
    }

    {
        skiatest::ReporterContext subtest(reporter, "unncached path");
        auto [view, colortype] = skgpu::ganesh::AsView(rContext, img, skgpu::Mipmapped::kNo,
                   GrImageTexGenPolicy::kNew_Uncached_Unbudgeted);
        REPORTER_ASSERT(reporter, view, "AsView returned falsey view");
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(PinnedImageTest_AsFragmentProcessor,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    GrRecordingContext* rContext = ctxInfo.directContext();
    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorMAGENTA); // arbitrary color

    sk_sp<SkImage> img = SkImages::PinnableRasterFromBitmap(bm);
    REPORTER_ASSERT(reporter, img, "PinnableImageFromBitmap returned null");

    SkTileMode tm[2] = {SkTileMode::kClamp, SkTileMode::kClamp};

    auto fp = skgpu::ganesh::AsFragmentProcessor(
            rContext, img.get(), SkSamplingOptions({1/3, 1/3}), tm,
            SkMatrix::I(), nullptr, nullptr);
    REPORTER_ASSERT(reporter, fp, "AsFragmentProcessor returned falsey processor");
}
