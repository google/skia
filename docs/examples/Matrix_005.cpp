// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8e45fe2dd52731bb2d4318686257e1d7
REG_FIDDLE(Matrix_005, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setAll(1, 0, 0,   0, 1, 0,    0, 0, 1);
    SkDebugf("identity flags hex: %0x decimal: %d\n", matrix.getType(), matrix.getType());
    matrix.setAll(1, 0, 0,   0, 1, 0,    0, 0, .5f);
    SkDebugf("set all  flags hex: %0x decimal: %d\n", matrix.getType(), matrix.getType());
}
}  // END FIDDLE
