// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(SkPath_cubicTo_example_parametric_animated, 512, 512, false, 0, 3) {
/*
        If the starting point is (x0, y0), then this curve is defined as the
        paramentric curve as `t` goes from 0 to 1:
          s := 1 - t
          x := (s * s * s * x0) +
               (3 * s * s * t * x1) +
               (3 * s * t * t * x2) +
               (t * t * t * x3)
          y := (s * s * s * y0) +
               (3 * s * s * t * y1) +
               (3 * s * t * t * y2) +
               (t * t * t * y3)

*/

SkPoint cubic(SkPoint p0, SkPoint p1, SkPoint p2, SkPoint p3, float t) {
    // a simple mathematical definition of cubic curve.
    // There are faster ways to calculate this.
    float s = 1 - t;
    return {(s * s * s * p0.x()) +
            (3 * s * s * t * p1.x()) +
            (3 * s * t * t * p2.x()) +
            (t * t * t * p3.x()),
            (s * s * s * p0.y()) +
            (3 * s * s * t * p1.y()) +
            (3 * s * t * t * p2.y()) +
            (t * t * t * p3.y())};
}

static SkPoint interpolate(SkPoint a, SkPoint b, float t) {
   return {SkScalarInterp(a.x(), b.x(), t),
           SkScalarInterp(a.y(), b.y(), t)};
}

void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255,255,255,255));

    SkPoint a{136,  64};
    SkPoint b{448, 448};
    SkPoint c{64,  448};
    SkPoint d{376, 64};

    SkPoint ab = interpolate(a, b, frame);
    SkPoint bc = interpolate(b, c, frame);
    SkPoint cd = interpolate(c, d, frame);
    SkPoint abc = interpolate(ab, bc, frame);
    SkPoint bcd = interpolate(bc, cd, frame);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);

    canvas->drawLine(ab, bc, paint);
    canvas->drawLine(bc, cd, paint);
    canvas->drawLine(abc, bcd, paint);

    paint.setStrokeWidth(3);
    canvas->drawLine(a, b, paint);
    canvas->drawLine(b, c, paint);
    canvas->drawLine(c, d, paint);

    paint.setStrokeWidth(5);
    paint.setColor(SkColorSetARGB(255, 0, 0, 255));
    SkPath cubicCurve;
    cubicCurve.moveTo(a);
    cubicCurve.cubicTo(b, c, d);
    canvas->drawPath(cubicCurve, paint);

    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 32);
    SkPaint textPaint;
    textPaint.setColor(SkColorSetARGB(255, 0, 255, 0));
    textPaint.setAntiAlias(true);

    sk_sp<SkTypeface> face(fontMgr->matchFamilyStyle("DejaVu Sans Mono", SkFontStyle()));
    canvas->drawString("a", a.x(),    a.y(), font, textPaint);
    canvas->drawString("b", b.x(),    b.y(), font, textPaint);
    canvas->drawString("c", c.x()-20, c.y(), font, textPaint);
    canvas->drawString("d", d.x(),    d.y(), font, textPaint);
    SkString msg = SkStringPrintf("%.4f", frame);
    textPaint.setColor(SkColorSetARGB(255, 204, 204, 204));
    canvas->drawString(msg.c_str(), 4, 36, font, textPaint);

    SkPaint pointPaint;
    pointPaint.setAntiAlias(true);
    pointPaint.setStrokeWidth(8);
    pointPaint.setStrokeCap(SkPaint::kRound_Cap);

    pointPaint.setColor(SkColorSetARGB(255, 255, 0, 0));
    canvas->drawPoint(interpolate(abc, bcd, frame), pointPaint);

    pointPaint.setColor(SkColorSetARGB(255, 0, 255, 0));
    pointPaint.setStrokeWidth(7);
    canvas->drawPoint(cubic(a, b, c, d, frame), pointPaint);
}
}  // END FIDDLE
