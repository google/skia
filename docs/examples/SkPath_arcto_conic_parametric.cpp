// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkPath_arcto_conic_parametric, 512, 512, false, 0) {
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
    paint.setStrokeWidth(2.5);

    SkPoint center = {256, 256};
    float r = 192;
    SkRect oval = {center.x() - r, center.y() - r, center.x() + r, center.y() + r};
    canvas->drawOval(oval, paint);
    float startAngle = 0;
    float sweepAngle = 179;

    SkPath arc;
    arc.arcTo(oval, startAngle, sweepAngle, false);

    paint.setStrokeWidth(5);
    paint.setColor(SkColorSetARGB(255, 0, 0, 255));
    canvas->drawPath(arc, paint);

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

    const int N = 8;
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
    paint.setColor(SK_ColorCYAN);
    paint.setStrokeWidth(1);
    canvas->drawPath(weightedQuadratic, paint);
}
}  // END FIDDLE
