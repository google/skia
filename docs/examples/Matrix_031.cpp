#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a18bc2e3607ac3a8e438bcb61fb13130
REG_FIDDLE(Matrix_031, 256, 48, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 8, 24, paint);
    SkMatrix matrix;
    matrix.setIdentity();
    matrix.setTranslateX(96);
    canvas->concat(matrix);
    canvas->drawString("x translate", 8, 24, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
