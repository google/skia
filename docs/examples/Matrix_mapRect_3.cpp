// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3b7b1f884437ab450f986234e4aec27f
REG_FIDDLE(Matrix_mapRect_3, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect{110, 50, 180, 100};
    SkMatrix matrix;
    matrix.setRotate(50, 28, 28);
    SkRect mapped = matrix.mapRect(rect);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);
    canvas->drawRect(mapped, paint);
    canvas->concat(matrix);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
