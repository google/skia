/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

using namespace sk_gpu_test;

#include "tools/gpu/GrContextFactory.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkImagePriv.h"

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
    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorRED);

    // We start off with the raster image being all red.
    sk_sp<SkImage> img = SkMakeImageFromRasterBitmap(bm, kNever_SkCopyPixelsMode);

    sk_sp<SkSurface> gpuSurface = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes, ii);
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
        SkImage_pinAsTexture(img.get(), rContext); // pin at blue

        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));

        bmCanvas.clear(SK_ColorBLUE);

        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));

        SkImage_unpinAsTexture(img.get(), rContext);
    }

    // once unpinned local changes will be picked up
    {
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorBLUE));
    }
}

// Deleting the context while there are still pinned images shouldn't result in a crash.
static void cleanup_test(skiatest::Reporter* reporter) {

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

                img = SkMakeImageFromRasterBitmap(bm, kNever_SkCopyPixelsMode);
                if (!SkImage_pinAsTexture(img.get(), dContext)) {
                    continue;
                }
                // Pinning on a second context should be blocked.
                REPORTER_ASSERT(reporter, !SkImage_pinAsTexture(img.get(), mockContext.get()));
            }

            // The context used to pin the image is gone at this point!
            // "context" isn't technically used in this call but it can't be null!
            // We don't really want to support this use case but it currently happens.
            SkImage_unpinAsTexture(img.get(), dContext);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PinnedImageTest, reporter, ctxInfo) {
    basic_test(reporter, ctxInfo.directContext());
    cleanup_test(reporter);
}
