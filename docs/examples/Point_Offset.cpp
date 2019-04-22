// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f0f24726df78a5d797bcf311e694a0a3
REG_FIDDLE(Point_Offset, 256, 256, false, 0) {
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
    SkPoint::Offset(points, SK_ARRAY_COUNT(points), { 1, 9 } );
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, SK_ARRAY_COUNT(points), points, paint);
}
}  // END FIDDLE
