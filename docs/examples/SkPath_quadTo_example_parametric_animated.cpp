// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(SkPath_quadTo_example_parametric_animated, 512, 512, false, 0, 4) {
// SkPath_quadTo_example_parametric_animated

SkPoint quad(SkPoint p0, SkPoint p1, SkPoint p2, float t) {
    float s = 1 - t;
    return {(s * s * p0.x()) + (2 * s * t * p1.x()) + (t * t * p2.x()),
            (s * s * p0.y()) + (2 * s * t * p1.y()) + (t * t * p2.y())};
}

/*

        If the starting point is (x0, y0), then this curve is defined as the
        paramentric curve as `t` goes from 0 to 1:

          s := 1 - t
          x := (s * s * x0) + (2 * s * t * x1) + (t * t * x2)
          y := (s * s * y0) + (2 * s * t * y1) + (t * t * y2)

*/

static SkPoint interpolate(SkPoint a, SkPoint b, float t) {
   return {SkScalarInterp(a.x(), b.x(), t),
           SkScalarInterp(a.y(), b.y(), t)};
}

static void draw_line(SkCanvas* canvas, SkPoint p0, SkPoint p1, const SkPaint& paint) {
    canvas->drawLine(p0.x(), p0.y(), p1.x(), p1.y(), paint);
}

static void draw_point(SkCanvas* canvas, SkPoint p, const SkPaint& paint) {
    canvas->drawPoint(p.x(), p.y(), paint);
}

void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255,255,255,255));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);

    SkPoint a{10, 100};
    SkPoint b{200, 400};
    SkPoint c{390, 100};

    SkPoint ab = interpolate(a, b, frame);
    SkPoint bc = interpolate(b, c, frame);

    draw_line(canvas, a, b, paint);
    draw_line(canvas, b, c, paint);

    paint.setStrokeWidth(1);
    draw_line(canvas, ab, bc, paint);
    paint.setStrokeWidth(5);

    paint.setColor(SkColorSetARGB(255,0,0,255));
    SkPath quadraticCurve;
    quadraticCurve.moveTo(a);
    quadraticCurve.quadTo(b, c);
    canvas->drawPath(quadraticCurve, paint);

    SkPaint textPaint;
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 32);
    textPaint.setAntiAlias(true);
    canvas->drawString("a", a.x(),    a.y(),    font, textPaint);
    canvas->drawString("b", b.x()+20, b.y()+20, font, textPaint);
    canvas->drawString("c", c.x(),    c.y(),    font, textPaint);

    SkPaint pointPaint;
    pointPaint.setAntiAlias(true);
    pointPaint.setStrokeWidth(8);
    pointPaint.setStrokeCap(SkPaint::kRound_Cap);
    pointPaint.setColor(SkColorSetARGB(255, 0, 255, 0));
    draw_point(canvas, quad(a, b, c, frame), pointPaint);
}
}  // END FIDDLE
