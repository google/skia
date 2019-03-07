// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Image_023, 256, 256, false, 4);
// HASH=1c6de6fe72b00b5be970f5f718363449
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(45);
    SkPaint paint;
    paint.setShader(image->makeShader(SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode,
                                      &matrix));
    canvas->drawPaint(paint);
}
}
