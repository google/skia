// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4eb2d95c9e9a66f05296e345bb68bd51
REG_FIDDLE(IPoint_addto_operator, 256, 64, false, 0) {
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
    points[1] += {1, 1};
    points[2] += {-1, -1};
    paint.setColor(SK_ColorRED);
    draw_lines(points, SK_ARRAY_COUNT(points), paint);
}
}  // END FIDDLE
