/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "src/core/SkAutoPixmapStorage.h"

#include "tests/Test.h"




static SkColor4f get_trans_black_expected_color(SkColorTypeComponentFlag components) {
    float r = 0, g = 0, b = 0, a = 0;

    if (!(components & kRed_SkColorTypeComponentFlag)) {
        r = 1;
    }
    if (!(components & kGreen_SkColorTypeComponentFlag)) {
        g = 1;
    }
    if (!(components & kBlue_SkColorTypeComponentFlag)) {
        b = 1;
    }
    if (!(components & kAlpha_SkColorTypeComponentFlag)) {
        a = 1;
    }

    return { r, g, b, a };
}

static SkColor4f get_opaque_white_expected_color(SkColorTypeComponentFlag components) {
    float r = 1, g = 1, b = 1;

    if (!(components & kRed_SkColorTypeComponentFlag)) {
        r = 0;
    }
    if (!(components & kGreen_SkColorTypeComponentFlag)) {
        g = 0;
    }
    if (!(components & kBlue_SkColorTypeComponentFlag)) {
        b = 0;
    }

    return { r, g, b, 1.0f };
}

struct TestCase {
    SkColorType              fColorType;
    SkAlphaType              fAlphaType;
    SkColorTypeComponentFlag fComponents;
    bool                     fIsOpaque;
    bool                     fCanMakeSurfaces;
};

TestCase gTests[] = {
    { kRGB_565_SkColorType, kOpaque_SkAlphaType, kRGB_SkColorTypeComponentFlags, true, true  },
    { kRG_88_SkColorType,   kOpaque_SkAlphaType, kRG_SkColorTypeComponentFlags,  true, false }
};

// For raster we expect:
//    SkSurface creation disallowed appropriately for some colorTypes
//    alpha always 1 for formats lacking an alpha channel
//    unused color channels always 0
//    alpha of source color is ignored for always opaque colorTypes
//    SkPixmap::erase works
//    writePixels/readPixels round trips for SkImages
//    making an SkImage and drawing it into F32 round trips
static void raster_tests(skiatest::Reporter* reporter, const TestCase& test) {

    SkImageInfo ii = SkImageInfo::Make(8, 8, test.fColorType, test.fAlphaType);

    uint32_t actualComponents = SkColorTypeComponentFlags(test.fColorType);
    REPORTER_ASSERT(reporter, test.fComponents == actualComponents);

    {
        auto ii = SkImageInfo::Make(32, 32, test.fColorType, test.fAlphaType);
        auto s = SkSurface::MakeRaster(ii);
        REPORTER_ASSERT(reporter, SkToBool(s) == test.fCanMakeSurfaces);
    }

    {
        SkAutoPixmapStorage pm;
        pm.alloc(ii);
        pm.erase(SkColors::kTransparent);
        SkColor actual = pm.getColor(0, 0);
        SkColor4f expected = get_trans_black_expected_color(test.fComponents);
        REPORTER_ASSERT(reporter, expected.toSkColor() == actual);
    }

    {
        SkAutoPixmapStorage pm;
        pm.alloc(ii);
        pm.erase(SkColors::kWhite);
        SkColor actual = pm.getColor(0, 0);
        SkColor4f expected = get_opaque_white_expected_color(test.fComponents);
        REPORTER_ASSERT(reporter, expected.toSkColor() == actual);
    }

}

// For GPU we expect:
//    SkSurface creation disallowed appropriately for some colorTypes
//    writePixels/readPixels round trips from SkImage
//    making SkImage and drawing it into F32 round trips
static void gpu_tests(GrContext* context, skiatest::Reporter* reporter, const TestCase& test) {

    {
        auto ii = SkImageInfo::Make(32, 32, test.fColorType, test.fAlphaType);
        auto s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);
        REPORTER_ASSERT(reporter, SkToBool(s) == test.fCanMakeSurfaces);
    }


}

DEF_TEST(ExtendedSkColorTypeTests_raster, reporter) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        raster_tests(reporter, gTests[i]);
    }
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ExtendedSkColorTypeTests_gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        gpu_tests(context, reporter, gTests[i]);
    }
}
