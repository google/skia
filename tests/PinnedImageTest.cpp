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
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
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
    sk_sp<SkImage> img = sk_image_factory::MakePinnableFromRasterBitmap(bm);
    REPORTER_ASSERT(reporter, img, "MakePinnableFromRasterBitmap returned null");

    sk_sp<SkSurface> gpuSurface = SkSurface::MakeRenderTarget(rContext, skgpu::Budgeted::kYes, ii);
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

    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory::ContextType ctxType = (GrContextFactory::ContextType) i;

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

                img = sk_image_factory::MakePinnableFromRasterBitmap(bm);
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
