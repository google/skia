#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6d6f2082fcf59d9f02bfb1758b87db69
REG_FIDDLE(Matrix_mapRadius, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkMatrix matrix;
    const SkPoint center = {108, 93};
    matrix.setScale(2, .5f, center.fX, center.fY);
    matrix.postRotate(45, center.fX, center.fY);
    const SkScalar circleRadius = 50;
    SkScalar mappedRadius = matrix.mapRadius(circleRadius);
    SkVector minorAxis, majorAxis;
    matrix.mapVector(0, circleRadius, &minorAxis);
    matrix.mapVector(circleRadius, 0, &majorAxis);
    SkString mappedArea;
    mappedArea.printf("area = %g", mappedRadius * mappedRadius);
    canvas->drawString(mappedArea, 145, 250, paint);
    canvas->drawString("mappedRadius", center.fX + mappedRadius + 3, center.fY, paint);
    paint.setColor(SK_ColorRED);
    SkString axArea;
    axArea.printf("area = %g", majorAxis.length() * minorAxis.length());
    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawString(axArea, 15, 250, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect({10, 200, 10 + majorAxis.length(), 200 + minorAxis.length()}, paint);
    paint.setColor(SK_ColorBLACK);
    canvas->drawLine(center.fX, center.fY, center.fX + mappedRadius, center.fY, paint);
    canvas->drawLine(center.fX, center.fY, center.fX, center.fY + mappedRadius, paint);
    canvas->drawRect({140, 180, 140 + mappedRadius, 180 + mappedRadius}, paint);
    canvas->concat(matrix);
    canvas->drawCircle(center.fX, center.fY, circleRadius, paint);
    paint.setColor(SK_ColorRED);
    canvas->drawLine(center.fX, center.fY, center.fX + circleRadius, center.fY, paint);
    canvas->drawLine(center.fX, center.fY, center.fX, center.fY + circleRadius, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
