// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_isFinite, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix = SkMatrix::Translate(SK_ScalarNaN, 0);
    matrix.dump();
    SkDebugf("matrix is finite: %s\n", matrix.isFinite() ? "true" : "false");
    SkDebugf("matrix %c= matrix\n", matrix == matrix ? '=' : '!');
}
}  // END FIDDLE
