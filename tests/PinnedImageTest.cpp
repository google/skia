/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

#if SK_SUPPORT_GPU

using namespace sk_gpu_test;

#include "GrContextFactory.h"

#include "SkCanvas.h"
#include "SkImagePriv.h"
#include "SkSurface.h"

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

static void basic_test(skiatest::Reporter* reporter, GrContext* context) {
    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorRED);

    // We start off with the raster image being all red.
    sk_sp<SkImage> img = SkMakeImageFromRasterBitmap(bm, kNever_SkCopyPixelsMode);

    sk_sp<SkSurface> gpuSurface = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii);
    SkCanvas* canvas = gpuSurface->getCanvas();

    // base case - nothing exciting just pinning & unpinning
    {
        SkImage_pinAsTexture(img.get(), context);
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorRED));
        SkImage_unpinAsTexture(img.get(), context);
    }

    bmCanvas.clear(SK_ColorGREEN);

    // pinned image shouldn't pick up changes to the underlying bitmap
    {
        SkImage_pinAsTexture(img.get(), context);
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));
        bmCanvas.clear(SK_ColorBLUE);
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorGREEN));
        SkImage_unpinAsTexture(img.get(), context);
    }

    bmCanvas.clear(SK_ColorCYAN);

    // once unpinned local changes will be picked up upon re-pinning
    {
        SkImage_pinAsTexture(img.get(), context);
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorCYAN));
        bmCanvas.clear(SK_ColorYELLOW);
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorCYAN));
        SkImage_unpinAsTexture(img.get(), context);

        SkImage_pinAsTexture(img.get(), context);
        canvas->drawImage(img, 0, 0);
        REPORTER_ASSERT(reporter, surface_is_expected_color(gpuSurface.get(), ii, SK_ColorYELLOW));
        SkImage_unpinAsTexture(img.get(), context);
    }
}

// Deleting the context while there are still pinned images shouldn't result in a crash.
static void cleanup_test(skiatest::Reporter* reporter) {

    const SkImageInfo ii = SkImageInfo::Make(64, 64, kN32_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    SkCanvas bmCanvas(bm);
    bmCanvas.clear(SK_ColorRED);

    for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
        GrContextFactory::ContextType ctxType = (GrContextFactory::ContextType) i;

        {
            sk_sp<SkImage> img;
            GrContext* context = nullptr;

            {
                GrContextFactory testFactory;
                ContextInfo info = testFactory.getContextInfo(ctxType);
                context = info.grContext();
                if (!context) {
                    continue;
                }

                img = SkMakeImageFromRasterBitmap(bm, kNever_SkCopyPixelsMode);
                if (!SkImage_pinAsTexture(img.get(), context)) {
                    continue;
                }
            }

            // The GrContext used to pin the image is gone at this point!
            // "context" isn't technically used in this call but it can't be null!
            // We don't really want to support this use case but it currently happens.
            SkImage_unpinAsTexture(img.get(), context);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(PinnedImageTest, reporter, ctxInfo) {
    basic_test(reporter, ctxInfo.grContext());
    cleanup_test(reporter);
}

#endif
