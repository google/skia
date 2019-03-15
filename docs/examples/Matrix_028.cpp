#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f040c6dd85a02e94eaca00d5c2832604
REG_FIDDLE(Matrix_028, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 12, 24, paint);
    SkMatrix matrix;
    matrix.setIdentity();
    matrix.setScaleY(3);
    canvas->concat(matrix);
    canvas->drawString("y scale", 12, 48, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
