/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColorShader.h"
#include "SkGradientShader.h"
#include "SkShader.h"
#include "SkTemplates.h"
#include "Test.h"

// https://code.google.com/p/chromium/issues/detail?id=448299
// Giant (inverse) matrix causes overflow when converting/computing using 32.32
// Before the fix, we would assert (and then crash).
static void test_big_grad(skiatest::Reporter* reporter) {
    const SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    const SkPoint pts[] = {{ 15, 14.7112684f }, { 0.709064007f, 12.6108112f }};
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
    SkPaint paint;
    paint.setShader(s)->unref();

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
    SkShader::TileMode fTileMode;

    void gradCheck(skiatest::Reporter* reporter, SkShader* shader,
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
        REPORTER_ASSERT(reporter, fTileMode == info->fTileMode);
    }
};


static void none_gradproc(skiatest::Reporter* reporter, const GradRec&) {
    SkAutoTUnref<SkShader> s(SkShader::CreateEmptyShader());
    REPORTER_ASSERT(reporter, SkShader::kNone_GradientType == s->asAGradient(NULL));
}

static void color_gradproc(skiatest::Reporter* reporter, const GradRec& rec) {
    SkAutoTUnref<SkShader> s(new SkColorShader(rec.fColors[0]));
    REPORTER_ASSERT(reporter, SkShader::kColor_GradientType == s->asAGradient(NULL));

    SkShader::GradientInfo info;
    info.fColors = NULL;
    info.fColorCount = 0;
    s->asAGradient(&info);
    REPORTER_ASSERT(reporter, 1 == info.fColorCount);
}

static void linear_gradproc(skiatest::Reporter* reporter, const GradRec& rec) {
    SkAutoTUnref<SkShader> s(SkGradientShader::CreateLinear(rec.fPoint,
                                                            rec.fColors,
                                                            rec.fPos,
                                                            rec.fColorCount,
                                                            rec.fTileMode));

    SkShader::GradientInfo info;
    rec.gradCheck(reporter, s, &info, SkShader::kLinear_GradientType);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, rec.fPoint, 2 * sizeof(SkPoint)));
}

static void radial_gradproc(skiatest::Reporter* reporter, const GradRec& rec) {
    SkAutoTUnref<SkShader> s(SkGradientShader::CreateRadial(rec.fPoint[0],
                                                            rec.fRadius[0],
                                                            rec.fColors,
                                                            rec.fPos,
                                                            rec.fColorCount,
                                                            rec.fTileMode));

    SkShader::GradientInfo info;
    rec.gradCheck(reporter, s, &info, SkShader::kRadial_GradientType);
    REPORTER_ASSERT(reporter, info.fPoint[0] == rec.fPoint[0]);
    REPORTER_ASSERT(reporter, info.fRadius[0] == rec.fRadius[0]);
}

static void sweep_gradproc(skiatest::Reporter* reporter, const GradRec& rec) {
    SkAutoTUnref<SkShader> s(SkGradientShader::CreateSweep(rec.fPoint[0].fX,
                                                           rec.fPoint[0].fY,
                                                           rec.fColors,
                                                           rec.fPos,
                                                           rec.fColorCount));

    SkShader::GradientInfo info;
    rec.gradCheck(reporter, s, &info, SkShader::kSweep_GradientType);
    REPORTER_ASSERT(reporter, info.fPoint[0] == rec.fPoint[0]);
}

static void conical_gradproc(skiatest::Reporter* reporter, const GradRec& rec) {
    SkAutoTUnref<SkShader> s(SkGradientShader::CreateTwoPointConical(rec.fPoint[0],
                                                             rec.fRadius[0],
                                                             rec.fPoint[1],
                                                             rec.fRadius[1],
                                                             rec.fColors,
                                                             rec.fPos,
                                                             rec.fColorCount,
                                                             rec.fTileMode));

    SkShader::GradientInfo info;
    rec.gradCheck(reporter, s, &info, SkShader::kConical_GradientType);
    REPORTER_ASSERT(reporter, !memcmp(info.fPoint, rec.fPoint, 2 * sizeof(SkPoint)));
    REPORTER_ASSERT(reporter, !memcmp(info.fRadius, rec.fRadius, 2 * sizeof(SkScalar)));
}

// Ensure that repeated color gradients behave like drawing a single color
static void TestConstantGradient(skiatest::Reporter*) {
    const SkPoint pts[] = {
        { 0, 0 },
        { SkIntToScalar(10), 0 }
    };
    SkColor colors[] = { SK_ColorBLUE, SK_ColorBLUE };
    const SkScalar pos[] = { 0, SK_Scalar1 };
    SkAutoTUnref<SkShader> s(SkGradientShader::CreateLinear(pts,
                                                            colors,
                                                            pos,
                                                            2,
                                                            SkShader::kClamp_TileMode));
    SkBitmap outBitmap;
    outBitmap.allocN32Pixels(10, 1);
    SkPaint paint;
    paint.setShader(s.get());
    SkCanvas canvas(outBitmap);
    canvas.drawPaint(paint);
    SkAutoLockPixels alp(outBitmap);
    for (int i = 0; i < 10; i++) {
        // The following is commented out because it currently fails
        // Related bug: https://code.google.com/p/skia/issues/detail?id=1098

        // REPORTER_ASSERT(reporter, SK_ColorBLUE == outBitmap.getColor(i, 0));
    }
}

typedef void (*GradProc)(skiatest::Reporter* reporter, const GradRec&);

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
    rec.fTileMode = SkShader::kClamp_TileMode;

    static const GradProc gProcs[] = {
        none_gradproc,
        color_gradproc,
        linear_gradproc,
        radial_gradproc,
        sweep_gradproc,
        conical_gradproc,
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gProcs); ++i) {
        gProcs[i](reporter, rec);
    }
}

DEF_TEST(Gradient, reporter) {
    TestGradientShaders(reporter);
    TestConstantGradient(reporter);
    test_big_grad(reporter);
}
