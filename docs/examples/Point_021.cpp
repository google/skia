// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8b4e79109e2381345258cb744881b20c
REG_FIDDLE(Point_021, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint points[] = { { 3, 1 }, { 4, 2 }, { 5, 1 }, { 7, 3 },
                         { 6, 4 }, { 7, 5 }, { 5, 7 },
                         { 4, 6 }, { 3, 7 }, { 1, 5 },
                         { 2, 4 }, { 1, 3 }, { 3, 1 } };
    canvas->scale(30, 15);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, SK_ARRAY_COUNT(points), points, paint);
    points[1] += {1, 1};
    points[2] += {-1, -1};
    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, SK_ARRAY_COUNT(points), points, paint);
}
}  // END FIDDLE
