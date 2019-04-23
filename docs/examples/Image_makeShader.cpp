// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1c6de6fe72b00b5be970f5f718363449
REG_FIDDLE(Image_makeShader, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(45);
    SkPaint paint;
    paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kMirror, &matrix));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
