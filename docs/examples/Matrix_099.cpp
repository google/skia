// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3fee4364929899649cf9efc37897e964
REG_FIDDLE(Matrix_099, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setScale(42, 24);
    SkDebugf("matrix.getMaxScale() %g\n", matrix.getMaxScale());
}
}  // END FIDDLE
