// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8d72a4818e5a9188348f6c08ab5d8a40
REG_FIDDLE(Matrix_dump, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setRotate(45);
    matrix.dump();
    SkMatrix nearlyEqual;
    nearlyEqual.setAll(0.7071f, -0.7071f, 0,   0.7071f, 0.7071f, 0,   0, 0, 1);
    nearlyEqual.dump();
    SkDebugf("matrix %c= nearlyEqual\n", matrix == nearlyEqual ? '=' : '!');
}
}  // END FIDDLE
