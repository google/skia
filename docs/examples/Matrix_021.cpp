// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Matrix_021, 256, 256, true, 0) {
// HASH=6236f7f2b91aff977a66ba2ee2558ca4
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setTranslate(42, 24);
    SkDebugf("matrix.getTranslateX() %c= 42\n", matrix.getTranslateX() == 42 ? '=' : '!');
}
}
