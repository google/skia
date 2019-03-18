#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c7177a6fbc1545be95a5ebca87e0cd0d
REG_FIDDLE(Matrix_setSkewX, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 12, 24, paint);
    SkMatrix matrix;
    matrix.setIdentity();
    matrix.setSkewX(-.7f);
    canvas->concat(matrix);
    canvas->drawString("x skew", 36, 48, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
