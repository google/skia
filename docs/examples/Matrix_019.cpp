// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6be5704506d029ffc91ba03b1d3e674b
REG_FIDDLE(Matrix_019, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setSkew(42, 24);
    SkDebugf("matrix.getSkewY() %c= 24\n", matrix.getSkewY() == 24 ? '=' : '!');
}
}  // END FIDDLE
