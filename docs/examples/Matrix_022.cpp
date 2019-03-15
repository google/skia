// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=08464e32d22421d2b254c71a84545ef5
REG_FIDDLE(Matrix_022, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setTranslate(42, 24);
    SkDebugf("matrix.getTranslateY() %c= 24\n", matrix.getTranslateY() == 24 ? '=' : '!');
}
}  // END FIDDLE
