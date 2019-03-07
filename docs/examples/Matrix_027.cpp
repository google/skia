#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a39dfed98c3c3c3a56be9ad59fe4e21e
REG_FIDDLE(Matrix_027, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 12, 24, paint);
    SkMatrix matrix;
    matrix.setIdentity();
    matrix.setScaleX(3);
    canvas->concat(matrix);
    canvas->drawString("x scale", 0, 48, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
