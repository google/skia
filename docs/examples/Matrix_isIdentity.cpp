// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=780ab376325b3cfa889ea26c0769ec11
REG_FIDDLE(Matrix_isIdentity, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setAll(1, 0, 0,   0, 1, 0,    0, 0, 1);
    SkDebugf("is identity: %s\n", matrix.isIdentity() ? "true" : "false");
    matrix.setAll(1, 0, 0,   0, 1, 0,    0, 0, 2);
    SkDebugf("is identity: %s\n", matrix.isIdentity() ? "true" : "false");
}
}  // END FIDDLE
