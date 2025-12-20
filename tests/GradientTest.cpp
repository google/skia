/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkColorPriv.h"
#include "src/shaders/SkShaderBase.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#endif

#include <cstdint>
#include <cstring>
#include <string>

// #if defined(SK_GRAPHITE)
// #include "include/gpu/graphite/Context.h"
// #include "include/gpu/graphite/Surface.h"
// #endif

struct GrContextOptions;

using namespace skia_private;

// https://code.google.com/p/chromium/issues/detail?id=448299
// Giant (inverse) matrix causes overflow when converting/computing using 32.32
// Before the fix, we would assert (and then crash).
static void test_big_grad(skiatest::Reporter* reporter) {
    const SkColor4f colors[] = { SkColors::kRed, SkColors::kBlue };
    const SkPoint pts[] = {{ 15, 14.7112684f }, { 0.709064007f, 12.6108112f }};
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}}));

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
    int fColorCount;
    const SkColor4f* fColors;
    const float* fPos;
    const SkPoint* fPoint;   // 2
    const float* fRadius; // 2
    SkTileMode fTileMode;

    SkGradient grad() const {
        SkSpan<const float> pos;
        if (fPos) {
            pos = {fPos, (size_t)fColorCount};
        }
        return {{{fColors, (size_t)fColorCount}, pos, fTileMode}, {}};
    }

    void gradCheck(skiatest::Reporter* reporter,
                   const sk_sp<SkShader>& shader,
                   SkShaderBase::GradientInfo* info,
                   SkShaderBase::GradientType gt,
                   const SkMatrix& localMatrix = SkMatrix::I()) const {
        AutoTMalloc<SkColor4f> colorStorage(fColorCount);
        AutoTMalloc<SkScalar> posStorage(fColorCount);

        info->fColorCount = fColorCount;
        info->fColors = colorStorage;
        info->fColorOffsets = posStorage.get();
        SkMatrix shaderLocalMatrix;
        REPORTER_ASSERT(reporter, as_SB(shader)->asGradient(info, &shaderLocalMatrix) == gt);
        REPORTER_ASSERT(reporter, shaderLocalMatrix == localMatrix);

        REPORTER_ASSERT(reporter, info->fColorCount == fColorCount);
        REPORTER_ASSERT(reporter,
                        !memcmp(info->fColors, fColors, fColorCount * sizeof(SkColor4f)));
        REPORTER_ASSERT(reporter,
                        !memcmp(info->fColorOffsets, fPos, fColorCount * sizeof(SkScalar)));
        REPORTER_ASSERT(reporter, fTileMode == (SkTileMode)info->fTileMode);
    }
};


static void none_gradproc(skiatest::Reporter* reporter, const GradRec&, const GradRec&) {
    sk_sp<SkShader> s(SkShaders::Empty());
    REPORTER_ASSERT(reporter, SkShaderBase::GradientType::kNone == as_SB(s)->asGradient());
}

static void color_gradproc(skiatest::Reporter* reporter, const GradRec& rec, const GradRec&) {
    sk_sp<SkShader> s(SkShaders::Color(rec.fColors[0], SkColorSpace::MakeSRGB()));
    REPORTER_ASSERT(reporter, SkShaderBase::GradientType::kNone == as_SB(s)->asGradient());
}

static void linear_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                            const GradRec& checkRec) {
    sk_sp<SkShader> s = SkShaders::LinearGradient(buildRec.fPoint, buildRec.grad());

    SkShaderBase::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShaderBase::GradientType::kLinear);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, checkRec.fPoint, 2 * sizeof(SkPoint)));
}

static void radial_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                            const GradRec& checkRec) {
    sk_sp<SkShader> s = SkShaders::RadialGradient(
            buildRec.fPoint[0], buildRec.fRadius[0], buildRec.grad());

    SkShaderBase::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShaderBase::GradientType::kRadial);
    REPORTER_ASSERT(reporter, info.fPoint[0] == checkRec.fPoint[0]);
    REPORTER_ASSERT(reporter, info.fRadius[0] == checkRec.fRadius[0]);
}

static void sweep_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                           const GradRec& checkRec) {
    sk_sp<SkShader> s = SkShaders::SweepGradient(buildRec.fPoint[0], buildRec.grad());

    SkShaderBase::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShaderBase::GradientType::kSweep);
    REPORTER_ASSERT(reporter, info.fPoint[0] == checkRec.fPoint[0]);
}

static void conical_gradproc(skiatest::Reporter* reporter, const GradRec& buildRec,
                             const GradRec& checkRec) {
    sk_sp<SkShader> s = SkShaders::TwoPointConicalGradient(
            buildRec.fPoint[0], buildRec.fRadius[0], buildRec.fPoint[1], buildRec.fRadius[1],
            buildRec.grad());

    SkShaderBase::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShaderBase::GradientType::kConical);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, checkRec.fPoint, 2 * sizeof(SkPoint)));
    REPORTER_ASSERT(reporter, !memcmp(info.fRadius, checkRec.fRadius, 2 * sizeof(SkScalar)));
}

static void linear_gradproc_matrix(skiatest::Reporter* reporter, const GradRec& buildRec,
                                   const GradRec& checkRec) {
    SkMatrix localMatrix = SkMatrix::RotateDeg(45, {100, 100});
    sk_sp<SkShader> s = SkShaders::LinearGradient(buildRec.fPoint, buildRec.grad(), &localMatrix);

    SkShaderBase::GradientInfo info;
    checkRec.gradCheck(reporter, s, &info, SkShaderBase::GradientType::kLinear, localMatrix);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, checkRec.fPoint, 2 * sizeof(SkPoint)));

    // Same but using a local matrix wrapper.
    s = SkShaders::LinearGradient(buildRec.fPoint, buildRec.grad());
    s = s->makeWithLocalMatrix(localMatrix);
    checkRec.gradCheck(reporter, s, &info, SkShaderBase::GradientType::kLinear, localMatrix);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, checkRec.fPoint, 2 * sizeof(SkPoint)));
}

// Ensure that repeated color gradients behave like drawing a single color
static void TestConstantGradient(skiatest::Reporter*) {
    const SkPoint pts[] = {
        { 0, 0 },
        { SkIntToScalar(10), 0 }
    };
    const SkColor4f colors[] = { SkColors::kBlue, SkColors::kBlue };
    const SkScalar pos[] = { 0, SK_Scalar1 };
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(pts, {{colors, pos, SkTileMode::kClamp}, {}}));
    SkBitmap outBitmap;
    outBitmap.allocN32Pixels(10, 1);
    SkCanvas canvas(outBitmap);
    canvas.drawPaint(paint);
    for (int i = 0; i < 10; i++) {
        // The following is commented out because it currently fails
        // Related bug: https://code.google.com/p/skia/issues/detail?id=1098

        // REPORTER_ASSERT(reporter, SkColors::kBlue == outBitmap.getColor(i, 0));
    }
}

typedef void (*GradProc)(skiatest::Reporter* reporter, const GradRec&, const GradRec&);

static void TestGradientShaders(skiatest::Reporter* reporter) {
    static const SkColor4f gColors[] = { SkColors::kRed,
                                         SkColors::kGreen,
                                         SkColors::kBlue };
    static const SkScalar gPos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
    static const SkPoint gPts[] = {
        { 0, 0 },
        { SkIntToScalar(10), SkIntToScalar(20) }
    };
    static const SkScalar gRad[] = { SkIntToScalar(1), SkIntToScalar(2) };

    GradRec rec;
    rec.fColorCount = std::size(gColors);
    rec.fColors = gColors;
    rec.fPos = gPos;
    rec.fPoint = gPts;
    rec.fRadius = gRad;
    rec.fTileMode = SkTileMode::kClamp;

    static const GradProc gProcs[] = {
        none_gradproc,
        color_gradproc,
        linear_gradproc,
        linear_gradproc_matrix,
        radial_gradproc,
        sweep_gradproc,
        conical_gradproc,
    };

    for (size_t i = 0; i < std::size(gProcs); ++i) {
        gProcs[i](reporter, rec, rec);
    }
}

static void test_nearly_vertical(skiatest::Reporter* reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 200)));

    const SkPoint pts[] = {{ 100, 50 }, { 100.0001f, 50000 }};
    const SkColor4f colors[] = { SkColors::kBlack, SkColors::kWhite };
    const SkScalar pos[] = { 0, 1 };
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(pts, {{colors, pos, SkTileMode::kClamp}, {}}));

    surface->getCanvas()->drawPaint(paint);
}

static void test_vertical(skiatest::Reporter* reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(200, 200)));

    const SkPoint pts[] = {{ 100, 50 }, { 100, 50 }};
    const SkColor4f colors[] = { SkColors::kBlack, SkColors::kWhite };
    const SkScalar pos[] = { 0, 1 };
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(pts, {{colors, pos, SkTileMode::kClamp}, {}}));

    surface->getCanvas()->drawPaint(paint);
}

// A linear gradient interval can, due to numerical imprecision (likely in the divide)
// finish an interval with the final fx not landing outside of [p0...p1].
// The old code had an assert which this test triggered.
// We now explicitly clamp the resulting fx value.
static void test_linear_fuzz(skiatest::Reporter* reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1300, 630)));

    const SkPoint pts[] = {{ 179.5f, -179.5f }, { 1074.5f, 715.5f }};
    const SkColor4f colors[] = { SkColors::kBlack, SkColors::kWhite, SkColors::kBlack, SkColors::kWhite };
    const SkScalar pos[] = {0, 0.200000003f, 0.800000012f, 1 };

    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(pts, {{colors, pos, SkTileMode::kClamp}, {}}));

    SkRect r = {0, 83, 1254, 620};
    surface->getCanvas()->drawRect(r, paint);
}

// https://bugs.chromium.org/p/skia/issues/detail?id=5023
// We should still shade pixels for which the radius is exactly 0.
static void test_two_point_conical_zero_radius(skiatest::Reporter* reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(5, 5)));
    surface->getCanvas()->clear(SkColors::kRed);

    const SkColor4f colors[] = { SkColors::kGreen, SkColors::kBlue };
    SkPaint p;
    p.setShader(SkShaders::TwoPointConicalGradient(
        SkPoint::Make(2.5f, 2.5f), 0,
        SkPoint::Make(3.0f, 3.0f), 10,
        {{colors, {}, SkTileMode::kClamp}, {}}));
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
    const SkColor4f colors[] = { SkColors::kRed, SkColors::kGreen };
    const SkPoint pts1[] = { SkPoint::Make(1001, 1000001), SkPoint::Make(1000.99f, 1000000) };

    p.setShader(SkShaders::LinearGradient(pts1, {{colors, {}, SkTileMode::kClamp}, {}}));

    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(50, 50)));
    surface->getCanvas()->scale(100, 100);
    surface->getCanvas()->drawPaint(p);

    const SkPoint pts2[] = { SkPoint::Make(10000.99f, 1000000), SkPoint::Make(10001, 1000001) };
    p.setShader(SkShaders::LinearGradient(pts2, {{colors, {}, SkTileMode::kClamp}, {}}));
    surface->getCanvas()->drawPaint(p);

    // Passes if we don't trigger asserts.
}

// http://crbug.com/636194
static void test_degenerate_linear(skiatest::Reporter*) {
    SkPaint p;
    const SkColor4f colors[] = { SkColors::kRed, SkColors::kGreen };
    const SkPoint pts[] = {
        SkPoint::Make(-46058024627067344430605278824628224.0f, 0),
        SkPoint::Make(SK_ScalarMax, 0)
    };

    p.setShader(SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}}));
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(50, 50)));
    surface->getCanvas()->drawPaint(p);

    // Passes if we don't trigger asserts.
}

#if defined(SK_GANESH)
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
    sk_sp<SkShader> gradient = SkShaders::LinearGradient(points,
                                                    {{colors, positions, SkTileMode::kMirror}, {}});

    // The degenerate gradient shouldn't be null
    REPORTER_ASSERT(r, SkToBool(gradient));
    // And it shouldn't crash when creating a fragment processor

    GrColorInfo dstColorInfo(GrColorType::kRGBA_8888, kPremul_SkAlphaType,
                             SkColorSpace::MakeSRGB());
    SkSurfaceProps props;
    GrMockOptions options;
    auto context = GrDirectContext::MakeMock(&options);

    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(context.get(),
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {800, 800},
                                                       SkSurfaceProps(),
                                                       /*label=*/{},
                                                       /* sampleCnt= */ 1,
                                                       skgpu::Mipmapped::kNo,
                                                       GrProtected::kNo,
                                                       kTopLeft_GrSurfaceOrigin);
    REPORTER_ASSERT(r, sdc);
    if (!sdc) {
        return;
    }

    GrFPArgs args(sdc.get(), &dstColorInfo, props, GrFPArgs::Scope::kDefault);
    GrFragmentProcessors::Make(gradient.get(), args, SkMatrix::I());
}
#endif

// "Interesting" fuzzer values.
static void test_linear_fuzzer(skiatest::Reporter*) {
    static const SkColor4f gColors0[] = { SkColor4f::FromColor(0x30303030),
                                          SkColor4f::FromColor(0x30303030) };
    static const SkColor4f gColors1[] = { SkColor4f::FromColor(0x30303030),
                                          SkColor4f::FromColor(0x30303030),
                                          SkColor4f::FromColor(0x30303030) };

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
        const SkColor4f*   fColors;
        const float*       fPos;
        size_t             fCount;
        SkTileMode         fTileMode;
        const float*       fLocalMatrix;
        const float*       fGlobalMatrix;

        SkGradient grad() const {
            SkSpan<const float> pos;
            if (fPos) {
                pos = {fPos, fCount};
            }
            return {{{fColors, fCount}, pos, fTileMode}, {}};
        }

    } gConfigs[] = {
        {
            {{0, -2.752941f}, {0, 0}},
            gColors0,
            nullptr,
            std::size(gColors0),
            SkTileMode::kClamp,
            gMatrix0,
            nullptr
        },
        {
            {{4.42539023e-39f, -4.42539023e-39f}, {9.78041162e-15f, 4.42539023e-39f}},
            gColors1,
            gPos1,
            std::size(gColors1),
            SkTileMode::kClamp,
            nullptr,
            gMatrix1
        },
        {
            {{4.42539023e-39f, 6.40969056e-10f}, {6.40969056e-10f, 1.49237238e-19f}},
            gColors1,
            gPos1,
            std::size(gColors1),
            SkTileMode::kClamp,
            nullptr,
            gMatrix2
        },
        {
            {{6.40969056e-10f, 6.40969056e-10f}, {6.40969056e-10f, -0.688235283f}},
            gColors0,
            nullptr,
            std::size(gColors0),
            SkTileMode::kClamp,
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
        sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::Make(
                100, 100, kN32_SkColorType, kPremul_SkAlphaType, sk_ref_sp(colorSpace)));
        SkCanvas* canvas = surface->getCanvas();

        for (const auto& config : gConfigs) {
            SkAutoCanvasRestore acr(canvas, false);
            std::optional<SkMatrix> localMatrix;
            if (config.fLocalMatrix) {
                localMatrix.emplace().set9(config.fLocalMatrix);
            }

            paint.setShader(SkShaders::LinearGradient(config.fPts,
                                                      config.grad(),
                                                      SkOptAddressOrNull(localMatrix)));
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
    static const SkColor4f gColors0[] = { SkColor4f::FromColor(0x30303030),
                                          SkColor4f::FromColor(0x30303030),
                                          SkColor4f::FromColor(0x30303030) };
    static const float   gPos0[] = { -47919293023455565225163489280.0f, 0, 1 };
    static const float gMatrix0[9] = {
        1.12116716e-13f,  0              ,  8.50489682e+16f,
        4.1917041e-41f ,  3.51369881e-23f, -2.54344271e-26f,
        9.61111907e+17f, -3.35263808e-29f, -1.35659403e+14f
    };
    static const struct {
        SkPoint            fCenter;
        const SkColor4f*   fColors;
        const float*       fPos;
        size_t             fCount;
        const float*       fGlobalMatrix;

        SkGradient grad() const {
            SkSpan<const float> pos;
            if (fPos) {
                pos = {fPos, fCount};
            }
            return {{{fColors, fCount}, pos, SkTileMode::kClamp}, {}};
        }

    } gConfigs[] = {
        {
            { 0, 0 },
            gColors0,
            gPos0,
            std::size(gColors0),
            gMatrix0
        },
    };

    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;

    for (const auto& config : gConfigs) {
        paint.setShader(SkShaders::SweepGradient(config.fCenter, config.grad()));

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

// Draw a sweep gradient in a translated canvas such that the colors in the center pixels of the
// gradient will be evaluated at x = 0. The gradient implementation must not call atan2(y, x) with
// x == 0, as this will result in undefined behavior and likely incorrect results.
// https://crbug.com/1468916
void test_sweep_gradient_zero_x(skiatest::Reporter* reporter, SkSurface* surface) {
    // The gradient drawn has yellow for the first half and blue for the second half, using hard
    // stops and running clockwise from (1, 0), so we should draw a rectangle with a blue top-half
    // and yellow bottom-half.
    constexpr float pts[4] = {0.0f, 0.5f, 0.5f, 1.0f};
    constexpr SkColor4f colors[4] = {SkColors::kYellow, SkColors::kYellow, SkColors::kBlue, SkColors::kBlue};
    SkCanvas* canvas = surface->getCanvas();
    canvas->save();
    canvas->translate(2.5f, 2.5f);
    SkPaint paint;
    paint.setShader(SkShaders::SweepGradient({0,0}, {{colors, pts, SkTileMode::kClamp}, {}}));
    canvas->drawRect(SkRect::MakeXYWH(-2.5f, -2.5f, 5.0f, 5.0f), paint);
    canvas->restore();

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surface->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Check the results.
    SkColor4f topColor = pixmap.getColor4f(2, 0);
    SkColor4f bottomColor = pixmap.getColor4f(2, 4);
    REPORTER_ASSERT(reporter, topColor == SkColors::kBlue);
    REPORTER_ASSERT(reporter, bottomColor == SkColors::kYellow);
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TestSweepGradientZeroXGanesh,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kApiLevel_202404) {
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(5, 5),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    GrDirectContext* context = contextInfo.directContext();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, ii);
    test_sweep_gradient_zero_x(reporter, surface.get());
}
#endif

// TODO: Fix this bug in Graphite as well.
// #if defined(SK_GRAPHITE)
// DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(TestSweepGradientZeroXGraphite, reporter, context,
//                                          CtsEnforcement::kNextRelease) {
//     using namespace skgpu::graphite;
//     SkImageInfo ii = SkImageInfo::Make(SkISize::Make(5, 5),
//                                        SkColorType::kRGBA_8888_SkColorType,
//                                        SkAlphaType::kPremul_SkAlphaType);
//     std::unique_ptr<Recorder> recorder = context->makeRecorder();
//     sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
//     test_sweep_gradient_zero_x(reporter, surface.get());
// }
// #endif

DEF_TEST(Gradient, reporter) {
    TestGradientShaders(reporter);
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
#if defined(SK_GANESH)
    test_unsorted_degenerate(reporter);
#endif
}
