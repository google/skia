// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f5ed382bd04fa7d50b2398cce2fca23a
REG_FIDDLE(Matrix_get, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setSkew(42, 24);
    SkDebugf("matrix.get(SkMatrix::kMSkewX) %c= 42\n",
             matrix.get(SkMatrix::kMSkewX) == 42 ? '=' : '!');
    SkDebugf("matrix.get(SkMatrix::kMSkewY) %c= 24\n",
             matrix.get(SkMatrix::kMSkewY) == 24 ? '=' : '!');
}
}  // END FIDDLE
