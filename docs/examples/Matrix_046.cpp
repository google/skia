// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=187e1d9228e2e4341ef820bd77b6fda9
REG_FIDDLE(Matrix_046, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setAntiAlias(true);
    SkRect rect = {20, 20, 100, 100};
    canvas->drawRect(rect, paint);
    paint.setColor(SK_ColorRED);
    SkMatrix matrix;
    matrix.setSinCos(.25f, .85f, rect.centerX(), rect.centerY());
    canvas->concat(matrix);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
