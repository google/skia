// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Matrix_017, 256, 256, true, 0) {
// HASH=ab746d9be63975041ae8e50cba84dc3d
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setScale(42, 24);
    SkDebugf("matrix.getScaleX() %c= 42\n", matrix.getScaleX() == 42 ? '=' : '!');
}
}
