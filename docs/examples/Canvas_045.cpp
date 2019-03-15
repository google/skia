// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=24b9cf7e6f9a08394e1e07413bd8733a
REG_FIDDLE(Canvas_045, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkFont font;
    canvas->scale(4, 6);
    canvas->drawString("truth", 2, 10, font, paint);
    SkMatrix matrix;
    matrix.setScale(2.8f, 6);
    canvas->setMatrix(matrix);
    canvas->drawString("consequences", 2, 20, font, paint);
}
}  // END FIDDLE
