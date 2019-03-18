// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0e03cd9f154603ed4b21ca56d69dae44
REG_FIDDLE(Matrix_invert, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    const SkPoint src[] = { { 10, 120}, {120, 120}, {120, 10}, {  10, 10} };
    const SkPoint dst[] = { {150, 120}, {200, 100}, {240, 30}, { 130, 40} };
    SkPaint paint;
    paint.setAntiAlias(true);
    SkMatrix matrix;
    matrix.setPolyToPoly(src, dst, 4);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 4, src, paint);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, 4, dst, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setStrokeWidth(3);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, dst, paint);
    if (matrix.invert(&matrix)) {
        canvas->concat(matrix);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, dst, paint);
    }
}
}  // END FIDDLE
