// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_mapXY, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkMatrix matrix;
    matrix.setRotate(60, 128, 128);
    SkPoint lines[] = {{50, 50}, {150, 50}, {150, 150}};
    for (size_t i = 0; i < std::size(lines); ++i) {
        SkPoint pt;
        matrix.mapXY(lines[i].fX, lines[i].fY, &pt);
        canvas->drawCircle(pt.fX, pt.fY, 3, paint);
    }
    canvas->concat(matrix);
    canvas->drawPoints(SkCanvas::kPolygon_PointMode, lines, paint);
}
}  // END FIDDLE
