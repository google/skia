// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bc6c6f6a5df770287120d87f81b922eb
REG_FIDDLE(Matrix_isFinite, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix = SkMatrix::MakeTrans(SK_ScalarNaN, 0);
    matrix.dump();
    SkDebugf("matrix is finite: %s\n", matrix.isFinite() ? "true" : "false");
    SkDebugf("matrix %c= matrix\n", matrix == matrix ? '=' : '!');
}
}  // END FIDDLE
