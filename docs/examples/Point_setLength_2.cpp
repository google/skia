// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3cc0662b6fbbee1fe3442a0acfece22c
REG_FIDDLE(Point_setLength_2, 256, 160, false, 0) {
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
        SkVector normal;
        normal.setLength(point.fX, point.fY, 100);
        paint.setStrokeWidth(10);
        paint.setColor(0x3fbf4512);
        canvas->drawLine(origin, normal, paint);
    }
}
}  // END FIDDLE
