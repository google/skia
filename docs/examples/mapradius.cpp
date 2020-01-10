// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(mapradius, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkMatrix matrix;
    const SkPoint center = {128, 103};
    matrix.setScale(2, .5f, center.fX, center.fY);
    matrix.postRotate(45, center.fX, center.fY);
    const SkScalar circleRadius = 60;
    SkScalar mappedRadius = matrix.mapRadius(circleRadius);
    SkVector minorAxis, majorAxis;
    matrix.mapVector(0, circleRadius, &minorAxis);
    matrix.mapVector(circleRadius, 0, &majorAxis);
    paint.setColor(SK_ColorRED);
    canvas->drawLine(40, 240, 40 + minorAxis.length(), 240, paint);
    canvas->drawLine(42 + minorAxis.length(), 240, 42 + minorAxis.length() + majorAxis.length(),
                     240, paint);
    paint.setColor(SK_ColorBLACK);
    canvas->drawLine(center.fX, center.fY, center.fX + mappedRadius, center.fY, paint);
    canvas->drawLine(center.fX, center.fY, center.fX, center.fY + mappedRadius, paint);
    canvas->drawLine(40, 220, 40 + mappedRadius, 220, paint);
    canvas->drawLine(42 + mappedRadius, 220, 42 + mappedRadius * 2, 220, paint);
    canvas->concat(matrix);
    canvas->drawCircle(center.fX, center.fY, circleRadius, paint);
    paint.setColor(SK_ColorRED);
    canvas->drawLine(center.fX, center.fY, center.fX + circleRadius, center.fY, paint);
    canvas->drawLine(center.fX, center.fY, center.fX, center.fY + circleRadius, paint);
}
}  // END FIDDLE
