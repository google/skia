// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e626e26bf557857b824aa7d03f723e0f
REG_FIDDLE(IPoint_subtract_operator, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    auto draw_lines = [=](const SkIPoint pts[], size_t count, SkPaint& paint) -> void {
        for (size_t i = 0; i < count - 1; ++i) {
            SkPoint p0, p1;
            p0.iset(pts[i]);
            p1.iset(pts[i + 1]);
            canvas->drawLine(p0, p1, paint);
        }
    };
    SkIPoint points[] = { { 3, 1 }, { 4, 2 }, { 5, 1 }, { 7, 3 } };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->scale(30, 15);
    draw_lines(points, SK_ARRAY_COUNT(points), paint);
    points[1] += points[0] - points[3];
    points[2] -= points[1] - points[0];
    paint.setColor(SK_ColorRED);
    draw_lines(points, SK_ARRAY_COUNT(points), paint);
}
}  // END FIDDLE
