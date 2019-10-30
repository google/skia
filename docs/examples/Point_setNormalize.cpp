// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3e4f147d143a388802484bf0d26534c2
REG_FIDDLE(Point_setNormalize, 256, 256, false, 0) {
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
        normal.setNormalize(point.fX, point.fY);
        normal *= 100;
        paint.setStrokeWidth(10);
        paint.setColor(0x3f4512bf);
        canvas->drawLine(origin, normal, paint);
    }
}
}  // END FIDDLE
