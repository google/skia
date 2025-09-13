// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_dirtyMatrixTypeCache, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const SkPoint p = {24, 42};
    SkMatrix matrix;
    matrix.setIdentity();
    SkDebugf("with identity matrix: x = %g\n", matrix.mapPoint(p).fX);
    SkScalar& skewRef = matrix[SkMatrix::kMSkewX];
    skewRef = 0;
    SkDebugf("after skew x mod:     x = %g\n", matrix.mapPoint(p).fX);
    skewRef = 1;
    SkDebugf("after 2nd skew x mod: x = %g\n", matrix.mapPoint(p).fX);
    matrix.dirtyMatrixTypeCache();
    SkDebugf("after dirty cache:    x = %g\n", matrix.mapPoint(p).fX);
}
}  // END FIDDLE
