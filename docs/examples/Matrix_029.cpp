#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b418d15df9829aefcc6aca93a37428bb
REG_FIDDLE(Matrix_setSkewY, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 12, 24, paint);
    SkMatrix matrix;
    matrix.setIdentity();
    matrix.setSkewY(.3f);
    canvas->concat(matrix);
    canvas->drawString("y skew", 12, 48, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
