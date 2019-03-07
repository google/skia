// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Matrix_026, 256, 256, true, 0) {
// HASH=1d400a92ca826cc89bcb88ea051f28c8
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setIdentity();
    SkDebugf("with identity matrix: x = %g\n", matrix.mapXY(24, 42).fX);
    matrix.set(SkMatrix::kMSkewX, 0);
    SkDebugf("after skew x mod:     x = %g\n", matrix.mapXY(24, 42).fX);
    matrix.set(SkMatrix::kMSkewX, 1);
    SkDebugf("after 2nd skew x mod: x = %g\n", matrix.mapXY(24, 42).fX);
}
}
