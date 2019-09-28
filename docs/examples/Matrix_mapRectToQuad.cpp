// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c69cd2a590b5733c3cbc92cb9ceed3f5
REG_FIDDLE(Matrix_mapRectToQuad, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkMatrix matrix;
    matrix.setRotate(60, 128, 128);
    SkRect rect = {50, 50, 150, 150};
    SkPoint pts[4];
    matrix.mapRectToQuad(pts, rect);
    for (int i = 0; i < 4; ++i) {
        canvas->drawCircle(pts[i].fX, pts[i].fY, 3, paint);
    }
    canvas->concat(matrix);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
