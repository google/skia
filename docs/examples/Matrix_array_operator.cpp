// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e8740493abdf0c6341762db9cee56b89
REG_FIDDLE(Matrix_array_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setScale(42, 24);
    SkDebugf("matrix[SkMatrix::kMScaleX] %c= 42\n", matrix[SkMatrix::kMScaleX] == 42 ? '=' : '!');
    SkDebugf("matrix[SkMatrix::kMScaleY] %c= 24\n", matrix[SkMatrix::kMScaleY] == 24 ? '=' : '!');
}
}  // END FIDDLE
