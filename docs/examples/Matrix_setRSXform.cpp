// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c3f5faddca466f78278b32b88fd5f5eb
REG_FIDDLE(Matrix_setRSXform, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setAntiAlias(true);
    SkRect rect = {20, 20, 100, 100};
    canvas->drawRect(rect, paint);
    paint.setColor(SK_ColorRED);
    SkMatrix matrix;
    matrix.setRSXform(SkRSXform::Make(.85f, .25f, rect.centerX(), rect.centerY()));
    canvas->concat(matrix);
    canvas->translate(-rect.centerX(), -rect.centerY());
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
