// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f4365ef332f51f7fd25040e0771ba9a2
REG_FIDDLE(Matrix_025, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setIdentity();
    SkDebugf("with identity matrix: x = %g\n", matrix.mapXY(24, 42).fX);
    SkScalar& skewRef = matrix[SkMatrix::kMSkewX];
    skewRef = 0;
    SkDebugf("after skew x mod:     x = %g\n", matrix.mapXY(24, 42).fX);
    skewRef = 1;
    SkDebugf("after 2nd skew x mod: x = %g\n", matrix.mapXY(24, 42).fX);
    matrix.dirtyMatrixTypeCache();
    SkDebugf("after dirty cache:    x = %g\n", matrix.mapXY(24, 42).fX);
}
}  // END FIDDLE
