// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkPath_arcto_conic_parametric2, 512, 512, false, 0) {
/** Add a weighted quadratic bezier from the last point, approaching control point
    (x1,y1), and ending at (x2,y2). If no moveTo() call has been made for
    this contour, the first point is automatically set to (0,0).
    If the starting point is (x0, y0), then this curve is defined as the
    paramentric curve as `t` goes from 0 to 1:
      s := 1 - t
      x := ((s * s * x0) + (w * 2 * s * t * x1) + (t * t * x2)) /
           ((s * s) + (w * 2 * s * t) + (t * t))
      y := ((s * s * y0) + (w * 2 * s * t * y1) + (t * t * y2)) /
           ((s * s) + (w * 2 * s * t) + (t * t))
    @param x1   The x-coordinate of the control point on a quadratic curve
    @param y1   The y-coordinate of the control point on a quadratic curve
    @param x2   The x-coordinate of the end point on a quadratic curve
    @param y2   The y-coordinate of the end point on a quadratic curve
    @param w    The weight of the control point (x1,y1)
*/

SkPoint conic(SkPoint p0, SkPoint p1, SkPoint p2, float w, float t) {
    float s = 1 - t;
    return {((s * s * p0.x()) + (2 * s * t * w * p1.x()) + (t * t * p2.x())) /
                    ((s * s) + (w * 2 * s * t) + (t * t)),
            ((s * s * p0.y()) + (2 * s * t * w * p1.y()) + (t * t * p2.y())) /
                    ((s * s) + (w * 2 * s * t) + (t * t))};
}

void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255, 255, 255, 255));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);

    SkPoint center = {256, 256};
    float r = 192;
    SkRect oval = {center.x() - r, center.y() - r, center.x() + r, center.y() + r};
    canvas->drawOval(oval, paint);
    float startAngle = 15;
    float sweepAngle = 75;

    SkPath arc;
    arc.arcTo(oval, startAngle, sweepAngle, false);

    SkPaint arcPaint(paint);
    arcPaint.setStrokeWidth(5);
    arcPaint.setColor(SkColorSetARGB(255, 0, 0, 255));
    canvas->drawPath(arc, arcPaint);

    SkPaint pointPaint;
    pointPaint.setAntiAlias(true);
    pointPaint.setStrokeWidth(8);
    pointPaint.setStrokeCap(SkPaint::kRound_Cap);
    pointPaint.setColor(SkColorSetARGB(255, 0, 255, 0));

    float finalAngle = startAngle + sweepAngle;
    float middleAngle = startAngle + 0.5f * sweepAngle;
    float weight = cos(SkDegreesToRadians(sweepAngle) / 2);
    SkPoint p0 = {r * SkScalarCos(SkDegreesToRadians(startAngle)),
                  r * SkScalarSin(SkDegreesToRadians(startAngle))};
    float d = r / weight;
    SkPoint p1 = {d * SkScalarCos(SkDegreesToRadians(middleAngle)),
                  d * SkScalarSin(SkDegreesToRadians(middleAngle))};
    SkPoint p2 = {r * SkScalarCos(SkDegreesToRadians(finalAngle)),
                  r * SkScalarSin(SkDegreesToRadians(finalAngle))};
    p0 += center;
    p1 += center;
    p2 += center;

    canvas->drawLine(p0.x(), p0.y(), p1.x(), p1.y(), paint);
    canvas->drawLine(p1.x(), p1.y(), p2.x(), p2.y(), paint);

    const int N = 16;
    for (int i = 0; i <= N; ++i) {
        SkPoint p = conic(p0, p1, p2, weight, (float)i / N);
        canvas->drawPoint(p.x(), p.y(), pointPaint);
    }
    pointPaint.setColor(SkColorSetARGB(255, 255, 0, 0));
    canvas->drawPoint(p0.x(), p0.y(), pointPaint);
    canvas->drawPoint(p1.x(), p1.y(), pointPaint);
    canvas->drawPoint(p2.x(), p2.y(), pointPaint);

    SkPath weightedQuadratic;
    weightedQuadratic.moveTo(p0);
    weightedQuadratic.conicTo(p1, p2, weight);
    paint.setColor(SK_ColorYELLOW);
    paint.setStrokeWidth(2.5);
    canvas->drawPath(weightedQuadratic, paint);
}
}  // END FIDDLE
