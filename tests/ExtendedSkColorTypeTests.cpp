/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrContextPriv.h"

#include "tests/Test.h"
#include "tests/TestUtils.h"

static constexpr int kSize = 32;


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
//    { kRGB_565_SkColorType, kOpaque_SkAlphaType, kRGB_SkColorTypeComponentFlags, true, true  },
    { kRG_88_SkColorType,   kOpaque_SkAlphaType, kRG_SkColorTypeComponentFlags,  true, false }
};

// For raster we expect:
//    SkSurface creation is disallowed appropriately for some colorTypes
//    alpha always 1 for formats lacking an alpha channel
//    unused color channels always 0
//    alpha of source color is ignored for always opaque colorTypes
//    SkPixmap::erase works
//    writePixels/readPixels round trips for SkImages
//    making an SkImage and drawing it into F32 round trips
static void raster_tests(skiatest::Reporter* reporter, const TestCase& test) {

    const SkImageInfo ii = SkImageInfo::Make(kSize, kSize, test.fColorType, test.fAlphaType);

    uint32_t actualComponents = SkColorTypeComponentFlags(test.fColorType);
    REPORTER_ASSERT(reporter, test.fComponents == actualComponents);

    {
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

static void compare_pixmaps(skiatest::Reporter* reporter,
                            const SkPixmap& expected, const SkPixmap& actual,
                            SkColorType nativeCT, const char* label) {
    const float tols[4] = {1.0f, 1.0f, 1.0f, 0};

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter, nativeCT, label](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "%s %s - mismatch at %d, %d (%f, %f, %f %f)",
                   nativeCT, label, x, y,
                   diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    compare_pixels(expected, actual, tols, error);
}

static void gpu_tests(GrContext* context, skiatest::Reporter* reporter, const TestCase& test) {

    const SkImageInfo nativeII = SkImageInfo::Make(kSize, kSize, test.fColorType, test.fAlphaType);
    const SkImageInfo f32Unpremul = SkImageInfo::Make(kSize, kSize, kRGBA_F32_SkColorType,
                                                      kUnpremul_SkAlphaType);
    const SkImageInfo f32Premul = SkImageInfo::Make(kSize, kSize, kRGBA_F32_SkColorType,
                                                    kPremul_SkAlphaType);

    // SkSurface creation is disallowed appropriately for some colorTypes
    {
        auto s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, nativeII);
        REPORTER_ASSERT(reporter, SkToBool(s) == test.fCanMakeSurfaces);
    }

    if (context->colorTypeSupportedAsImage(test.fColorType)) {
        SkAutoPixmapStorage nativeExpected;

        nativeExpected.alloc(nativeII);
        nativeExpected.erase(SkColors::kWhite);

        // create an initialized backend texture
        GrBackendTexture backendTex = context->priv().createBackendTexture(&nativeExpected, 1,
                                                                           GrRenderable::kNo,
                                                                           GrProtected::kNo);
        REPORTER_ASSERT(reporter, backendTex.isValid());

        auto img = SkImage::MakeFromTexture(context, backendTex, kTopLeft_GrSurfaceOrigin,
                                            test.fColorType, test.fAlphaType, nullptr);
        REPORTER_ASSERT(reporter, SkToBool(img));

        // SkImage::readPixels with the same colorType as the source pixels round trips
        {
            SkAutoPixmapStorage nativeActual;
            nativeActual.alloc(nativeII);
            nativeActual.erase(SkColors::kTransparent);

            REPORTER_ASSERT(reporter, img->readPixels(nativeActual, 0, 0));

            compare_pixmaps(reporter, nativeExpected, nativeActual,
                            test.fColorType, "readback to native CT");
        }

        {
            SkAutoPixmapStorage f32Expected, f32Actual;

            f32Expected.alloc(f32Unpremul);
            f32Expected.erase(get_opaque_white_expected_color(test.fComponents));

            f32Actual.alloc(f32Unpremul);

            // reading back to F32 should always work
            {
                f32Actual.erase(SkColors::kTransparent);
                REPORTER_ASSERT(reporter, img->readPixels(f32Actual, 0, 0));

                compare_pixmaps(reporter, f32Expected, f32Actual,
                                test.fColorType, "readback to F32");
            }

            // drawing a native SkImage works appropriately (as assessed by reading back from an
            // F32 surface)
            {
                auto s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, f32Premul);
                REPORTER_ASSERT(reporter, SkToBool(s));

                {
                    SkCanvas* c = s->getCanvas();
                    c->drawImage(img, 0, 0);
                }

                f32Actual.erase(SkColors::kTransparent);
                REPORTER_ASSERT(reporter, s->readPixels(f32Actual, 0, 0));

                compare_pixmaps(reporter, f32Expected, f32Actual,
                                test.fColorType, "drawn to F32");
            }
        }

        img.reset();
        context->flush();
        context->deleteBackendTexture(backendTex);
    }

}

DEF_TEST(ExtendedSkColorTypeTests_raster, reporter) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        raster_tests(reporter, gTests[i]);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ExtendedSkColorTypeTests_gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTests); ++i) {
        gpu_tests(context, reporter, gTests[i]);
    }
}
