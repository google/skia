/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColor.h"
#include "SkGradientShader.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkSize.h"
#include "SkString.h"

#include "gm.h"

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorYELLOW
};

// These annoying defines are necessary, because the only other alternative
// is to use SkIntToScalar(...) everywhere.
static const SkScalar sZero = 0;
static const SkScalar sHalf = SK_ScalarHalf;
static const SkScalar sOne = SK_Scalar1;

// These arrays define the gradient stop points
// as x1, y1, x2, y2 per gradient to draw.
static const SkPoint linearPts[][2] = {
    {{sZero, sZero}, {sOne,  sZero}},
    {{sZero, sZero}, {sZero, sOne}},
    {{sOne,  sZero}, {sZero, sZero}},
    {{sZero, sOne},  {sZero, sZero}},

    {{sZero, sZero}, {sOne,  sOne}},
    {{sOne,  sOne},  {sZero, sZero}},
    {{sOne,  sZero}, {sZero, sOne}},
    {{sZero, sOne},  {sOne,  sZero}}
};

static const SkPoint radialPts[][2] = {
    {{sZero, sHalf}, {sOne,  sHalf}},
    {{sHalf, sZero}, {sHalf, sOne}},
    {{sOne,  sHalf}, {sZero, sHalf}},
    {{sHalf, sOne},  {sHalf, sZero}},

    {{sZero, sZero}, {sOne,  sOne}},
    {{sOne,  sOne},  {sZero, sZero}},
    {{sOne,  sZero}, {sZero, sOne}},
    {{sZero, sOne},  {sOne,  sZero}}
};

// These define the pixels allocated to each gradient image.
static const SkScalar TESTGRID_X = SkIntToScalar(200);
static const SkScalar TESTGRID_Y = SkIntToScalar(200);

static const int IMAGES_X = 4;             // number of images per row

static SkShader* make_linear_gradient(const SkPoint pts[2], const SkMatrix& localMatrix) {
    return SkGradientShader::CreateLinear(pts, gColors, nullptr, SK_ARRAY_COUNT(gColors),
                                          SkShader::kClamp_TileMode, 0, &localMatrix);
}

static SkShader* make_radial_gradient(const SkPoint pts[2], const SkMatrix& localMatrix) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    float radius = (center - pts[0]).length();
    return SkGradientShader::CreateRadial(center, radius, gColors, nullptr, SK_ARRAY_COUNT(gColors),
                                          SkShader::kClamp_TileMode, 0, &localMatrix);
}

static void draw_gradients(SkCanvas* canvas,
                           SkShader* (*makeShader)(const SkPoint[2], const SkMatrix&),
                           const SkPoint ptsArray[][2], int numImages) {
    // Use some nice prime numbers for the rectangle and matrix with
    // different scaling along the x and y axes (which is the bug this
    // test addresses, where incorrect order of operations mixed up the axes)
    SkRect rectGrad = {
        SkIntToScalar(43),  SkIntToScalar(61),
        SkIntToScalar(181), SkIntToScalar(167) };
    SkMatrix shaderMat;
    shaderMat.setScale(rectGrad.width(), rectGrad.height());
    shaderMat.postTranslate(rectGrad.left(), rectGrad.top());

    canvas->save();
    for (int i = 0; i < numImages; i++) {
        // Advance line downwards if necessary.
        if (i % IMAGES_X == 0 && i != 0) {
            canvas->restore();
            canvas->translate(0, TESTGRID_Y);
            canvas->save();
        }

        // Setup shader and draw.
        SkAutoTUnref<SkShader> shader(makeShader(*ptsArray, shaderMat));

        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(rectGrad, paint);

        // Advance to next position.
        canvas->translate(TESTGRID_X, 0);
        ptsArray++;
    }
    canvas->restore();
}

DEF_SIMPLE_GM_BG(gradient_matrix, canvas, 800, 800,
                 sk_tool_utils::color_to_565(0xFFDDDDDD)) {
        draw_gradients(canvas, &make_linear_gradient,
                      linearPts, SK_ARRAY_COUNT(linearPts));

        canvas->translate(0, TESTGRID_Y);

        draw_gradients(canvas, &make_radial_gradient,
                      radialPts, SK_ARRAY_COUNT(radialPts));
}
