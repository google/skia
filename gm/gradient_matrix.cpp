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

// these arrays define the gradient stop points
// as x1, y1, x2, y2 per gradient to draw
static const SkPoint linearPts[][2] = {
    {{0, 0}, {1, 0}},
    {{0, 0}, {0, 1}},
    {{1, 0}, {0, 0}},
    {{0, 1}, {0, 0}},

    {{0, 0}, {1, 1}},
    {{1, 1}, {0, 0}},
    {{1, 0}, {0, 1}},
    {{0, 1}, {1, 0}}
};

static const SkScalar TESTGRID_X = 200;    // pixels allocated to each image in x dimension
static const SkScalar TESTGRID_Y = 200;    // pixels allocated to each image in y dimension

static const int IMAGES_X = 4;             // number of images per row

static SkShader* make_linear_gradient(const SkPoint pts[2]) {
    return SkGradientShader::CreateLinear(pts, gColors, NULL, SK_ARRAY_COUNT(gColors),
                                          SkShader::kClamp_TileMode, NULL);
}

static void draw_gradients(SkCanvas* canvas, SkShader* (*makeShader)(const SkPoint[2]),
                    const SkPoint ptsArray[][2], int numImages) {
    // Use some nice prime numbers for the rectangle and matrix with
    // different scaling along the x and y axes (which is the bug this
    // test addresses, where incorrect order of operations mixed up the axes)
    SkRect rectGrad = { 43, 61, 181, 167 };
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
        SkAutoTUnref<SkShader> shader(makeShader(*ptsArray));
        shader->setLocalMatrix(shaderMat);

        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(rectGrad, paint);

        // Advance to next position.
        canvas->translate(TESTGRID_X, 0);
        ptsArray++;
    }
    canvas->restore();
}

namespace skiagm {

class GradientMatrixGM : public GM {
public:
    GradientMatrixGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() SK_OVERRIDE {
        return SkString("gradient_matrix");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(800, 800);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw_gradients(canvas, &make_linear_gradient,
                      linearPts, SK_ARRAY_COUNT(linearPts));
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new GradientMatrixGM; )
}
