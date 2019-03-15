// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cbe7db206ece825aa3b9b7c3256aeaf0
REG_FIDDLE(Point_015, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint points[] = { { 60, -110 }, { 90, 10 }, { 120, -110 }, { 180, -50 } };
    const SkPoint origin = {0, 0};
    canvas->translate(30, 140);
    for (auto point : points) {
        paint.setStrokeWidth(1);
        paint.setColor(SK_ColorBLACK);
        canvas->drawLine(origin, point, paint);
        SkVector normal = point;
        normal.setLength(100);
        paint.setStrokeWidth(10);
        paint.setColor(0x3f45bf12);
        canvas->drawLine(origin, normal, paint);
    }
}
}  // END FIDDLE
