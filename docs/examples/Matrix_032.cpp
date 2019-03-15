#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=34e3c70a72b836abf7f4858d35eecc98
REG_FIDDLE(Matrix_032, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 8, 24, paint);
    SkMatrix matrix;
    matrix.setIdentity();
    matrix.setTranslateY(24);
    canvas->concat(matrix);
    canvas->drawString("y translate", 8, 24, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
