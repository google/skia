#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ccfc734aff2ddea0b097c83f5621de5e
REG_FIDDLE(Matrix_041, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    canvas->drawString("normal", 8, 24, paint);
    SkMatrix matrix;
    matrix.setTranslate({96, 24});
    canvas->concat(matrix);
    canvas->drawString("translate", 8, 24, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
