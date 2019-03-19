// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8c28db3add9cd0177225088f6df6bbb5
REG_FIDDLE(Matrix_setRotate, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setAntiAlias(true);
    SkRect rect = {20, 20, 100, 100};
    canvas->drawRect(rect, paint);
    paint.setColor(SK_ColorRED);
    SkMatrix matrix;
    matrix.setRotate(25, rect.centerX(), rect.centerY());
    canvas->concat(matrix);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
