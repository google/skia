// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=428ca171ae3bd0d3f992458ac598b97b
REG_FIDDLE(Matrix_078, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(35, 128, 128);
    const int count = 4;
    SkPoint pts[count];
    matrix.mapRectToQuad(pts, {40, 70, 180, 220} );
    SkPaint paint;
    paint.setARGB(77, 23, 99, 154);
    for (int i = 0; i < 5; ++i) {
        canvas->drawPoints(SkCanvas::kPolygon_PointMode, count, pts, paint);
        matrix.mapPoints(pts, count);
    }
}
}  // END FIDDLE
