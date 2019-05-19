// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=35b3bc675779de043706ae4817ee950c
REG_FIDDLE(Point_multiply_operator, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint points[] = { { 3, 1 }, { 4, 2 }, { 5, 1 }, { 7, 3 },
                         { 6, 4 }, { 7, 5 }, { 5, 7 },
                         { 4, 6 }, { 3, 7 }, { 1, 5 },
                         { 2, 4 }, { 1, 3 }, { 3, 1 } };
    canvas->scale(15, 10);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, SK_ARRAY_COUNT(points), points, paint);
    for (auto& point : points) {
        point = point * 1.5f;
    }
    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, SK_ARRAY_COUNT(points), points, paint);
}
}  // END FIDDLE
