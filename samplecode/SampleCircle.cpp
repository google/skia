/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "samplecode/Sample.h"

// ensure that we don't accidentally screw up the bounds when the oval is
// fractional, and the impl computes the center and radii, and uses them to
// reconstruct the edges of the circle.
// see bug# 1504910
static void test_circlebounds(SkCanvas*) {
    SkRect r = { 1.39999998f, 1, 21.3999996f, 21 };
    SkPath p;
    p.addOval(r);
    SkASSERT(r == p.getBounds());
}

class CircleView : public Sample {
    SkString name() override { return SkString("Circles"); }

    void circle(SkCanvas* canvas, int width, bool aa) {
        SkPaint paint;

        paint.setAntiAlias(aa);
        if (width < 0) {
            paint.setStyle(SkPaint::kFill_Style);
        } else {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SkIntToScalar(width));
        }
        canvas->drawCircle(0, 0, 9.0f, paint);
        if (false) { // avoid bit rot, suppress warning
            test_circlebounds(canvas);
        }
    }

    void drawSix(SkCanvas* canvas, SkScalar dx, SkScalar dy) {
        for (int width = -1; width <= 1; width++) {
            canvas->save();
            circle(canvas, width, false);
            canvas->translate(0, dy);
            circle(canvas, width, true);
            canvas->restore();
            canvas->translate(dx, 0);
        }
    }

    static void make_poly(SkPath* path, int n) {
        if (n <= 0) {
            return;
        }
        path->incReserve(n + 1);
        path->moveTo(SK_Scalar1, 0);
        SkScalar step = SK_ScalarPI * 2 / n;
        SkScalar angle = 0;
        for (int i = 1; i < n; i++) {
            angle += step;
            path->lineTo(SkScalarCos(angle), SkScalarSin(angle));
        }
        path->close();
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        SkMatrix matrix;
        matrix.setScale(SkIntToScalar(100), SkIntToScalar(100));
        matrix.postTranslate(SkIntToScalar(200), SkIntToScalar(200));
        canvas->concat(matrix);
        for (int n = 3; n < 20; n++) {
            SkPath path;
            make_poly(&path, n);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(SkIntToScalar(10) * (n - 3));
            canvas->translate(-SK_Scalar1, 0);
            canvas->drawPath(path, paint);
        }
    }
};

DEF_SAMPLE( return new CircleView(); )
