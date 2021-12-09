/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColorInfo.h"
#include "src/shaders/SkColorShader.h"
#include "tests/Test.h"

// https://code.google.com/p/chromium/issues/detail?id=448299
// Giant (inverse) matrix causes overflow when converting/computing using 32.32
// Before the fix, we would assert (and then crash).
static void test_big_grad(skiatest::Reporter* reporter) {
    const SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    const SkPoint pts[] = {{ 15, 14.7112684f }, { 0.709064007f, 12.6108112f }};
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));

    SkBitmap bm;
    bm.allocN32Pixels(2000, 1);
    SkCanvas c(bm);

    const SkScalar affine[] = {
        1.06608627e-06f, 4.26434525e-07f, 6.2855f, 2.6611f, 273.4393f, 244.0046f
    };
    SkMatrix matrix;
    matrix.setAffine(affine);
    c.concat(matrix);

    c.drawPaint(paint);
}

struct GradRec {
    int             fColorCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
    const SkPoint*  fPoint;   // 2
    const SkScalar* fRadius; // 2
    SkTileMode      fTileMode;

    void gradCheck(skiatest::Reporter* reporter, const sk_sp<SkShader>& shader,
                   SkShader::GradientInfo* info,
                   SkShader::GradientType gt) const {
        SkAutoTMalloc<SkColor> colorStorage(fColorCount);
        SkAutoTMalloc<SkScalar> posStorage(fColorCount);

        info->fColorCount = fColorCount;
        info->fColors = colorStorage;
        info->fColorOffsets = posStorage.get();
        REPORTER_ASSERT(reporter, shader->asAGradient(info) == gt);

        REPORTER_ASSERT(reporter, info->fColorCount == fColorCount);
        REPORTER_ASSERT(reporter,
                        !memcmp(info->fColors, fColors, fColorCount * sizeof(SkColor)));
        REPORTER_ASSERT(reporter,
                        !memcmp(info->fColorOffsets, fPos, fColorCount * sizeof(SkScalar)));
        REPORTER_ASSERT(reporter, fTileMode == (SkTileMode)info->fTileMode);
    }
};


static void none_gradproc(skiatest::Reporter* reporter, const GradRec&, const GradRec&) {
    sk_sp<SkShader> s(SkShaders::Empty());
    REPORTER_ASSERT(reporter, SkShader::kNone_GradientType == s->asAGradient(nullptr));
}

static void color_gradproc(skiatest::Reporter* reporter, const GradRec& rec, const GradRec&) {
    sk_sp<SkShader> s(new SkColorShader(rec.fColors[0]));
    REPORTER_ASSERT(reporter, SkShader::kColor_GradientType == s->asAGradient(nullptr));

    SkShader::GradientInfo info;
    info.fColors = nullptr;
    info.fColorCount = 0;
    s->asAGradient(&info);
    REPORTER_ASSERT(reporter, 1 == info.fColorCount);
}

static void linear_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                            const GradRec& checkRec) {
    sk_sp<SkShader> s(SkGradientShader::MakeLinear(buildRec.fPoint, buildRec.fColors, buildRec.fPos,
                                                   buildRec.fColorCount, buildRec.fTileMode));

    SkShader::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShader::kLinear_GradientType);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, checkRec.fPoint, 2 * sizeof(SkPoint)));
}

static void radial_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                            const GradRec& checkRec) {
    sk_sp<SkShader> s(SkGradientShader::MakeRadial(buildRec.fPoint[0], buildRec.fRadius[0],
                                                   buildRec.fColors, buildRec.fPos,
                                                   buildRec.fColorCount, buildRec.fTileMode));

    SkShader::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShader::kRadial_GradientType);
    REPORTER_ASSERT(reporter, info.fPoint[0] == checkRec.fPoint[0]);
    REPORTER_ASSERT(reporter, info.fRadius[0] == checkRec.fRadius[0]);
}

static void sweep_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                           const GradRec& checkRec) {
    sk_sp<SkShader> s(SkGradientShader::MakeSweep(buildRec.fPoint[0].fX, buildRec.fPoint[0].fY,
                                                  buildRec.fColors, buildRec.fPos,
                                                  buildRec.fColorCount));

    SkShader::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShader::kSweep_GradientType);
    REPORTER_ASSERT(reporter, info.fPoint[0] == checkRec.fPoint[0]);
}

static void conical_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                             const GradRec& checkRec) {
    sk_sp<SkShader> s(SkGradientShader::MakeTwoPointConical(buildRec.fPoint[0],
                                                            buildRec.fRadius[0],
                                                            buildRec.fPoint[1],
                                                            buildRec.fRadius[1],
                                                            buildRec.fColors,
                                                            buildRec.fPos,
                                                            buildRec.fColorCount,
                                                            buildRec.fTileMode));

    SkShader::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShader::kConical_GradientType);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, checkRec.fPoint, 2 * sizeof(SkPoint)));
    REPORTER_ASSERT(reporter, !memcmp(info.fRadius, checkRec.fRadius, 2 * sizeof(SkScalar)));
}

// Ensure that repeated color gradients behave like drawing a single color
static void TestConstantGradient(skiatest::Reporter*) {
    const SkPoint pts[] = {
        { 0, 0 },
        { SkIntToScalar(10), 0 }
    };
    SkColor colors[] = { SK_ColorBLUE, SK_ColorBLUE };
    const SkScalar pos[] = { 0, SK_Scalar1 };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 2, SkTileMode::kClamp));
    SkBitmap outBitmap;
    outBitmap.allocN32Pixels(10, 1);
    SkCanvas canvas(outBitmap);
    canvas.drawPaint(paint);
    for (int i = 0; i < 10; i++) {
        // The following is commented out because it currently fails
        // Related bug: https://code.google.com/p/skia/issues/detail?id=1098

        // REPORTER_ASSERT(reporter, SK_ColorBLUE == outBitmap.getColor(i, 0));
    }
}

typedef void (*GradProc)(skiatest::Reporter* reporter, const GradRec&, const GradRec&);

static void TestGradientShaders(skiatest::Reporter* reporter) {
    static const SkColor gColors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    static const SkScalar gPos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
    static const SkPoint gPts[] = {
        { 0, 0 },
        { SkIntToScalar(10), SkIntToScalar(20) }
    };
    static const SkScalar gRad[] = { SkIntToScalar(1), SkIntToScalar(2) };

    GradRec rec;
    rec.fColorCount = SK_ARRAY_COUNT(gColors);
    rec.fColors = gColors;
    rec.fPos = gPos;
    rec.fPoint = gPts;
    rec.fRadius = gRad;
    rec.fTileMode = SkTileMode::kClamp;

    static const GradProc gProcs[] = {
        none_gradproc,
        color_gradproc,
        linear_gradproc,
        radial_gradproc,
        sweep_gradproc,
        conical_gradproc,
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gProcs); ++i) {
        gProcs[i](reporter, rec, rec);
    }
}

static void TestGradientOptimization(skiatest::Reporter* reporter) {
    static const struct {
        GradProc fProc;
        bool     fIsClampRestricted;
    } gProcInfo[] = {
        { linear_gradproc , false },
        { radial_gradproc , false },
        { sweep_gradproc  , true  }, // sweep is funky in that it always pretends to be kClamp.
        { conical_gradproc, false },
    };

    static const SkColor   gC_00[] = { 0xff000000, 0xff000000 };
    static const SkColor   gC_01[] = { 0xff000000, 0xffffffff };
    static const SkColor   gC_11[] = { 0xffffffff, 0xffffffff };
    static const SkColor  gC_001[] = { 0xff000000, 0xff000000, 0xffffffff };
    static const SkColor  gC_011[] = { 0xff000000, 0xffffffff, 0xffffffff };
    static const SkColor gC_0011[] = { 0xff000000, 0xff000000, 0xffffffff, 0xffffffff };

    static const SkScalar   gP_01[] = { 0, 1 };
    static const SkScalar  gP_001[] = { 0,   0, 1 };
    static const SkScalar  gP_011[] = { 0,   1, 1 };
    static const SkScalar  gP_0x1[] = { 0, .5f, 1 };
    static const SkScalar gP_0011[] = { 0, 0, 1, 1 };

    static const SkPoint    gPts[] = { {0, 0}, {1, 1} };
    static const SkScalar gRadii[] = { 1, 2 };

    static const struct {
        const SkColor*  fCol;
        const SkScalar* fPos;
        int             fCount;

        const SkColor*  fExpectedCol;
        const SkScalar* fExpectedPos;
        int             fExpectedCount;
        bool            fRequiresNonClamp;
    } gTests[] = {
        { gC_001,  gP_001, 3,  gC_01,  gP_01, 2, false },
        { gC_001,  gP_011, 3,  gC_00,  gP_01, 2, true  },
        { gC_001,  gP_0x1, 3, gC_001, gP_0x1, 3, false },
        { gC_001, nullptr, 3, gC_001, gP_0x1, 3, false },

        { gC_011,  gP_001, 3,  gC_11,  gP_01, 2, true  },
        { gC_011,  gP_011, 3,  gC_01,  gP_01, 2, false },
        { gC_011,  gP_0x1, 3, gC_011, gP_0x1, 3, false },
        { gC_011, nullptr, 3, gC_011, gP_0x1, 3, false },

        { gC_0011, gP_0011, 4, gC_0011, gP_0011, 4, false },
    };

    const SkTileMode modes[] = {
        SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kMirror,
        // TODO: add kDecal_TileMode when it is implemented
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gProcInfo); ++i) {
        for (auto mode : modes) {
            if (gProcInfo[i].fIsClampRestricted && mode != SkTileMode::kClamp) {
                continue;
            }

            for (size_t t = 0; t < SK_ARRAY_COUNT(gTests); ++t) {
                GradRec rec;
                rec.fColorCount = gTests[t].fCount;
                rec.fColors     = gTests[t].fCol;
                rec.fPos        = gTests[t].fPos;
                rec.fTileMode   = mode;
                rec.fPoint      = gPts;
                rec.fRadius     = gRadii;

                GradRec expected = rec;
                if (!gTests[t].fRequiresNonClamp || mode != SkTileMode::kClamp) {
                    expected.fColorCount = gTests[t].fExpectedCount;
                    expected.fColors     = gTests[t].fExpectedCol;
                    expected.fPos        = gTests[t].fExpectedPos;
                }

                gProcInfo[i].fProc(reporter, rec, expected);
            }
        }
    }
}

static void test_nearly_vertical(skiatest::Reporter* reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(200, 200));

    const SkPoint pts[] = {{ 100, 50 }, { 100.0001f, 50000 }};
    const SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
    const SkScalar pos[] = { 0, 1 };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 2, SkTileMode::kClamp));

    surface->getCanvas()->drawPaint(paint);
}

static void test_vertical(skiatest::Reporter* reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(200, 200));

    const SkPoint pts[] = {{ 100, 50 }, { 100, 50 }};
    const SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
    const SkScalar pos[] = { 0, 1 };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 2, SkTileMode::kClamp));

    surface->getCanvas()->drawPaint(paint);
}

// A linear gradient interval can, due to numerical imprecision (likely in the divide)
// finish an interval with the final fx not landing outside of [p0...p1].
// The old code had an assert which this test triggered.
// We now explicitly clamp the resulting fx value.
static void test_linear_fuzz(skiatest::Reporter* reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(1300, 630));

    const SkPoint pts[] = {{ 179.5f, -179.5f }, { 1074.5f, 715.5f }};
    const SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE, SK_ColorBLACK, SK_ColorWHITE };
    const SkScalar pos[] = {0, 0.200000003f, 0.800000012f, 1 };

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 4, SkTileMode::kClamp));

    SkRect r = {0, 83, 1254, 620};
    surface->getCanvas()->drawRect(r, paint);
}

// https://bugs.chromium.org/p/skia/issues/detail?id=5023
// We should still shade pixels for which the radius is exactly 0.
static void test_two_point_conical_zero_radius(skiatest::Reporter* reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(5, 5));
    surface->getCanvas()->clear(SK_ColorRED);

    const SkColor colors[] = { SK_ColorGREEN, SK_ColorBLUE };
    SkPaint p;
    p.setShader(SkGradientShader::MakeTwoPointConical(
        SkPoint::Make(2.5f, 2.5f), 0,
        SkPoint::Make(3.0f, 3.0f), 10,
        colors, nullptr, SK_ARRAY_COUNT(colors), SkTileMode::kClamp));
    surface->getCanvas()->drawPaint(p);

    // r == 0 for the center pixel.
    // verify that we draw it (no red bleed)
    SkPMColor centerPMColor;
    surface->readPixels(SkImageInfo::MakeN32Premul(1, 1), &centerPMColor, sizeof(SkPMColor), 2, 2);
    REPORTER_ASSERT(reporter, SkGetPackedR32(centerPMColor) == 0);
}

// http://crbug.com/599458
static void test_clamping_overflow(skiatest::Reporter*) {
    SkPaint p;
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
    const SkPoint pts1[] = { SkPoint::Make(1001, 1000001), SkPoint::Make(1000.99f, 1000000) };

    p.setShader(SkGradientShader::MakeLinear(pts1, colors, nullptr, 2, SkTileMode::kClamp));

    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
    surface->getCanvas()->scale(100, 100);
    surface->getCanvas()->drawPaint(p);

    const SkPoint pts2[] = { SkPoint::Make(10000.99f, 1000000), SkPoint::Make(10001, 1000001) };
    p.setShader(SkGradientShader::MakeLinear(pts2, colors, nullptr, 2, SkTileMode::kClamp));
    surface->getCanvas()->drawPaint(p);

    // Passes if we don't trigger asserts.
}

// http://crbug.com/636194
static void test_degenerate_linear(skiatest::Reporter*) {
    SkPaint p;
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
    const SkPoint pts[] = {
        SkPoint::Make(-46058024627067344430605278824628224.0f, 0),
        SkPoint::Make(SK_ScalarMax, 0)
    };

    p.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
    surface->getCanvas()->drawPaint(p);

    // Passes if we don't trigger asserts.
}

// http://crbug.com/1149216
static void test_unsorted_degenerate(skiatest::Reporter* r) {
    // Passes if a valid solid color is computed for the degenerate gradient
    // (unsorted positions are fixed during regular gradient construction, so this ensures the
    // same fixing happens for degenerate gradients as well). If they aren't fixed, this test
    // case produces a negative alpha, which asserts during SkPMColor4f::isOpaque().
    const SkColor4f colors[] = { {0.f, 0.f, 0.f, 0.f},
                                 {0.00784314f, 0.f, 0.f, 0.0627451f},
                                 {0.f, 0.00392157f, 0.f, 0.f} };
    const SkScalar positions[] = {0.00753367f, 8.54792e-44f, 1.46955e-39f};

    const SkPoint points[] { { 0.f, 0.f }, { 1e-20f, -1e-8f }}; // must be degenerate
    // Use kMirror to go through average color stop calculation, vs. kClamp which would pick a color
    sk_sp<SkShader> gradient = SkGradientShader::MakeLinear(points, colors, nullptr, positions, 3,
                                                            SkTileMode::kMirror);

    // The degenerate gradient shouldn't be null
    REPORTER_ASSERT(r, SkToBool(gradient));
    // And it shouldn't crash when creating a fragment processor

    SkMatrixProvider provider(SkMatrix::I());
    GrColorInfo dstColorInfo(GrColorType::kRGBA_8888, kPremul_SkAlphaType,
                             SkColorSpace::MakeSRGB());
    GrMockOptions options;
    auto context = GrDirectContext::MakeMock(&options);

    GrFPArgs args(context.get(), provider, &dstColorInfo);
    as_SB(gradient)->asFragmentProcessor(args);
}

// "Interesting" fuzzer values.
static void test_linear_fuzzer(skiatest::Reporter*) {
    static const SkColor gColors0[] = { 0x30303030, 0x30303030 };
    static const SkColor gColors1[] = { 0x30303030, 0x30303030, 0x30303030 };

    static const SkScalar gPos1[]   = { 0, 0, 1 };

    static const SkScalar gMatrix0[9] = {
        6.40969056e-10f, 0              , 6.40969056e-10f,
        0              , 4.42539023e-39f, 6.40969056e-10f,
        0              , 0              , 1
    };
    static const SkScalar gMatrix1[9] = {
        -2.75294113f    , 6.40969056e-10f,  6.40969056e-10f,
         6.40969056e-10f, 6.40969056e-10f, -3.32810161e+24f,
         6.40969056e-10f, 6.40969056e-10f,  0
    };
    static const SkScalar gMatrix2[9] = {
        7.93481258e+17f, 6.40969056e-10f, 6.40969056e-10f,
        6.40969056e-10f, 6.40969056e-10f, 6.40969056e-10f,
        6.40969056e-10f, 6.40969056e-10f, 0.688235283f
    };
    static const SkScalar gMatrix3[9] = {
        1.89180674e+11f,     6.40969056e-10f, 6.40969056e-10f,
        6.40969056e-10f,     6.40969056e-10f, 6.40969056e-10f,
        6.40969056e-10f, 11276.0469f        , 8.12524808e+20f
    };

    static const struct {
        SkPoint            fPts[2];
        const SkColor*     fColors;
        const SkScalar*    fPos;
        int                fCount;
        SkTileMode         fTileMode;
        uint32_t           fFlags;
        const SkScalar*    fLocalMatrix;
        const SkScalar*    fGlobalMatrix;
    } gConfigs[] = {
        {
            {{0, -2.752941f}, {0, 0}},
            gColors0,
            nullptr,
            SK_ARRAY_COUNT(gColors0),
            SkTileMode::kClamp,
            0,
            gMatrix0,
            nullptr
        },
        {
            {{4.42539023e-39f, -4.42539023e-39f}, {9.78041162e-15f, 4.42539023e-39f}},
            gColors1,
            gPos1,
            SK_ARRAY_COUNT(gColors1),
            SkTileMode::kClamp,
            0,
            nullptr,
            gMatrix1
        },
        {
            {{4.42539023e-39f, 6.40969056e-10f}, {6.40969056e-10f, 1.49237238e-19f}},
            gColors1,
            gPos1,
            SK_ARRAY_COUNT(gColors1),
            SkTileMode::kClamp,
            0,
            nullptr,
            gMatrix2
        },
        {
            {{6.40969056e-10f, 6.40969056e-10f}, {6.40969056e-10f, -0.688235283f}},
            gColors0,
            nullptr,
            SK_ARRAY_COUNT(gColors0),
            SkTileMode::kClamp,
            0,
            gMatrix3,
            nullptr
        },
    };

    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    SkColorSpace* colorSpaces[] = {
        nullptr,     // hits the legacy gradient impl
        srgb.get(),  // triggers 4f/raster-pipeline
    };

    SkPaint paint;

    for (const SkColorSpace* colorSpace : colorSpaces) {

        sk_sp<SkSurface> surface = SkSurface::MakeRaster(SkImageInfo::Make(100, 100,
                                                                           kN32_SkColorType,
                                                                           kPremul_SkAlphaType,
                                                                           sk_ref_sp(colorSpace)));
        SkCanvas* canvas = surface->getCanvas();

        for (const auto& config : gConfigs) {
            SkAutoCanvasRestore acr(canvas, false);
            SkTLazy<SkMatrix> localMatrix;
            if (config.fLocalMatrix) {
                localMatrix.init();
                localMatrix->set9(config.fLocalMatrix);
            }

            paint.setShader(SkGradientShader::MakeLinear(config.fPts,
                                                         config.fColors,
                                                         config.fPos,
                                                         config.fCount,
                                                         config.fTileMode,
                                                         config.fFlags,
                                                         localMatrix.getMaybeNull()));
            if (config.fGlobalMatrix) {
                SkMatrix m;
                m.set9(config.fGlobalMatrix);
                canvas->save();
                canvas->concat(m);
            }

            canvas->drawPaint(paint);
        }
    }
}

static void test_sweep_fuzzer(skiatest::Reporter*) {
    static const SkColor gColors0[] = { 0x30303030, 0x30303030, 0x30303030 };
    static const SkScalar   gPos0[] = { -47919293023455565225163489280.0f, 0, 1 };
    static const SkScalar gMatrix0[9] = {
        1.12116716e-13f,  0              ,  8.50489682e+16f,
        4.1917041e-41f ,  3.51369881e-23f, -2.54344271e-26f,
        9.61111907e+17f, -3.35263808e-29f, -1.35659403e+14f
    };
    static const struct {
        SkPoint            fCenter;
        const SkColor*     fColors;
        const SkScalar*    fPos;
        int                fCount;
        const SkScalar*    fGlobalMatrix;
    } gConfigs[] = {
        {
            { 0, 0 },
            gColors0,
            gPos0,
            SK_ARRAY_COUNT(gColors0),
            gMatrix0
        },
    };

    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(100, 100);
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;

    for (const auto& config : gConfigs) {
        paint.setShader(SkGradientShader::MakeSweep(config.fCenter.x(),
                                                    config.fCenter.y(),
                                                    config.fColors,
                                                    config.fPos,
                                                    config.fCount));

        SkAutoCanvasRestore acr(canvas, false);
        if (config.fGlobalMatrix) {
            SkMatrix m;
            m.set9(config.fGlobalMatrix);
            canvas->save();
            canvas->concat(m);
        }
        canvas->drawPaint(paint);
    }
}

DEF_TEST(Gradient, reporter) {
    TestGradientShaders(reporter);
    TestGradientOptimization(reporter);
    TestConstantGradient(reporter);
    test_big_grad(reporter);
    test_nearly_vertical(reporter);
    test_vertical(reporter);
    test_linear_fuzz(reporter);
    test_two_point_conical_zero_radius(reporter);
    test_clamping_overflow(reporter);
    test_degenerate_linear(reporter);
    test_linear_fuzzer(reporter);
    test_sweep_fuzzer(reporter);
    test_unsorted_degenerate(reporter);
}
