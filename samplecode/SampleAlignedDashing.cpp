/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "samplecode/Sample.h"

#include "experimental/aligned_dashing/SkAlignedDashPathEffect.h"

static void make_unit_star(SkPath* path, int n) {
    SkScalar rad = -SK_ScalarPI / 2;
    const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

    path->moveTo(0, -SK_Scalar1);
    for (int i = 1; i < n; i++) {
        rad += drad;
        path->lineTo(SkScalarCos(rad), SkScalarSin(rad));
    }
    path->close();
}

static void make_path_star(SkPath* path, const SkRect& bounds) {
    make_unit_star(path, 5);
    SkMatrix matrix;
    matrix.setRectToRect(path->getBounds(), bounds, SkMatrix::kCenter_ScaleToFit);
    path->transform(matrix);
}

//////////////////////////////////////////////////////////////////////////////

class AlignedDashingSample : public Sample {
    SkString name() override { return SkString("aligneddashing"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(4);
        paint.setAntiAlias(true);

        canvas->translate(20, 20);

        drawLines(canvas, paint);
        canvas->translate(0, 110);

        drawRects(canvas, paint);
        canvas->translate(0, 110);

        drawOvals(canvas, paint);
        canvas->translate(0, 110);

        drawQuads(canvas, paint);
        canvas->translate(0, 110);

        drawStars(canvas, paint);
        canvas->translate(0, 110);

        drawMultiContours(canvas, paint);
        canvas->translate(0, 110);
    }

private:
    const std::vector<std::vector<SkScalar>> fAllIntervals = {{2, 40, 20}, {4, 40, 10, 5, 10}};

    sk_sp<SkPathEffect> makeDashes(size_t intervalsIdx) const {
        const int count = fAllIntervals[intervalsIdx][0];
        return SkAlignedDashPathEffect::Make(&fAllIntervals[intervalsIdx][1], count);
    }

    void drawLines(SkCanvas* canvas, const SkPaint& paint) {
        canvas->save();
        SkPaint p(paint);
        for (SkPoint lr : std::vector<SkPoint>{{30, 30}, {75, 75}}) {
            for (size_t i = 0; i < fAllIntervals.size(); i++) {
                p.setPathEffect(makeDashes(i));
                canvas->drawLine(0, 0, lr.fX, lr.fY, p);
                canvas->translate(75, 0);
            }
        }
        canvas->restore();
    }

    void drawRects(SkCanvas* canvas, const SkPaint& paint) {
        canvas->save();
        SkPaint p(paint);
        for (SkSize size : std::vector<SkSize>{{100, 30}, {30, 100}, {75, 75}}) {
            for (size_t i = 0; i < fAllIntervals.size(); i++) {
                p.setPathEffect(makeDashes(i));
                canvas->drawRect(SkRect::MakeXYWH(0, 0, size.fWidth, size.fHeight), p);
                canvas->translate(120, 0);
            }
        }
        canvas->restore();
    }

    void drawOvals(SkCanvas* canvas, const SkPaint& paint) {
        canvas->save();
        SkPaint p(paint);
        for (SkSize size : std::vector<SkSize>{{100, 30}, {30, 100}, {75, 75}}) {
            for (size_t i = 0; i < fAllIntervals.size(); i++) {
                p.setPathEffect(makeDashes(i));
                canvas->drawOval(SkRect::MakeXYWH(0, 0, size.fWidth, size.fHeight), p);
                canvas->translate(120, 0);
            }
        }
        canvas->restore();
    }

    void drawQuads(SkCanvas* canvas, const SkPaint& paint) {
        canvas->save();
        SkPaint p(paint);
        for (SkScalar width : {100, 25}) {
            for (size_t i = 0; i < fAllIntervals.size(); i++) {
                p.setPathEffect(makeDashes(i));

                SkPath path;
                path.moveTo(0, 50);
                path.quadTo(width / 2, 100, width, 50);
                path.quadTo(3 * width / 2, 0, 2 * width, 50);
                path.quadTo(5 * width / 2, 100, 3 * width, 50);

                canvas->drawPath(path, p);
                canvas->translate(3 * width + 20, 0);
            }
        }
        canvas->restore();
    }

    void drawStars(SkCanvas* canvas, const SkPaint& paint) {
        canvas->save();
        SkPaint p(paint);
        for (size_t i = 0; i < fAllIntervals.size(); i++) {
            p.setPathEffect(makeDashes(i));
            const float size = 100;
            SkPath path;
            make_path_star(&path, SkRect::MakeLTRB(0, 0, size, size));
            canvas->drawPath(path, p);
            canvas->translate(size + 20, 0);
        }
        canvas->restore();
    }

    void drawMultiContours(SkCanvas* canvas, const SkPaint& paint) {
        canvas->save();
        SkPaint p(paint);
        for (bool closeSecond : {true, false}) {
            SkPath path;
            path.moveTo(0, 0);
            path.lineTo(30, 30);
            path.lineTo(60, 0);
            path.close();

            path.moveTo(80, 0);
            path.lineTo(110, 30);
            path.lineTo(140, 0);
            if (closeSecond) path.close();

            for (size_t i = 0; i < fAllIntervals.size(); i++) {
                p.setPathEffect(makeDashes(i));
                canvas->drawPath(path, p);
                canvas->translate(150, 0);
            }
        }
        canvas->restore();
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE(return new AlignedDashingSample;)
