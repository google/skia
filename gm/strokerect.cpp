/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"

#include <float.h>

#define STROKE_WIDTH    SkIntToScalar(20)

static void draw_path(SkCanvas* canvas, const SkPath& path, const SkRect& rect,
                      SkPaint::Join join, int doFill) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(doFill ? SkPaint::kStrokeAndFill_Style : SkPaint::kStroke_Style);

    paint.setColor(SK_ColorGRAY);
    paint.setStrokeWidth(STROKE_WIDTH);
    paint.setStrokeJoin(join);
    canvas->drawRect(rect, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorRED);
    canvas->drawPath(path, paint);

    paint.setStrokeWidth(3);
    paint.setStrokeJoin(SkPaint::kMiter_Join);
    int n = path.countPoints();
    SkAutoTArray<SkPoint> points(n);
    path.getPoints(points.get(), n);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, n, points.get(), paint);
}

/*
 *  Test calling SkStroker for rectangles. Cases to cover:
 *
 *  geometry: normal, small (smaller than stroke-width), empty, inverted
 *  joint-type for the corners
 */
class StrokeRectGM : public skiagm::GM {
public:
    StrokeRectGM() {}

protected:

    SkString onShortName() override {
        return SkString("strokerect");
    }

    SkISize onISize() override {
        return SkISize::Make(1400, 740);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        canvas->translate(STROKE_WIDTH*3/2, STROKE_WIDTH*3/2);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(STROKE_WIDTH);

        constexpr SkPaint::Join gJoins[] = {
            SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join
        };

        constexpr SkScalar W = 80;
        constexpr SkScalar H = 80;
        constexpr SkRect gRects[] = {
            { 0, 0, W, H },
            { W, 0, 0, H },
            { 0, H, W, 0 },
            { 0, 0, STROKE_WIDTH, H },
            { 0, 0, W, STROKE_WIDTH },
            { 0, 0, STROKE_WIDTH/2, STROKE_WIDTH/2 },
            { 0, 0, W, 0 },
            { 0, 0, 0, H },
            { 0, 0, 0, 0 },
            { 0, 0, W, FLT_EPSILON },
            { 0, 0, FLT_EPSILON, H },
            { 0, 0, FLT_EPSILON, FLT_EPSILON },
        };

        for (int doFill = 0; doFill <= 1; ++doFill) {
            for (size_t i = 0; i < SK_ARRAY_COUNT(gJoins); ++i) {
                SkPaint::Join join = gJoins[i];
                paint.setStrokeJoin(join);

                SkAutoCanvasRestore acr(canvas, true);
                for (size_t j = 0; j < SK_ARRAY_COUNT(gRects); ++j) {
                    const SkRect& r = gRects[j];

                    SkPath path, fillPath;
                    path.addRect(r);
                    paint.getFillPath(path, &fillPath);
                    draw_path(canvas, fillPath, r, join, doFill);

                    canvas->translate(W + 2 * STROKE_WIDTH, 0);
                }
                acr.restore();
                canvas->translate(0, H + 2 * STROKE_WIDTH);
            }
            paint.setStyle(SkPaint::kStrokeAndFill_Style);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM(return new StrokeRectGM;)

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  Exercise rect-stroking (which is specialized from paths) when the resulting stroke-width is
 *  non-square. See https://bugs.chromium.org/p/skia/issues/detail?id=5408
 */
DEF_SIMPLE_GM(strokerect_anisotropic_5408, canvas, 200, 50) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(6);

    canvas->scale(10, 1);
    SkRect r = SkRect::MakeXYWH(5, 20, 10, 10);
    canvas->drawRect(r, p);
}
