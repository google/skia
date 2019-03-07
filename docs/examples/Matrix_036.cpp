// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=379fc375e011050b54ed9df83c0996a7
REG_FIDDLE(Matrix_036, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix = SkMatrix::MakeRectToRect({0, 0, 1, 1}, {3, 4, 7, 9},
                                               SkMatrix::kFill_ScaleToFit);
    SkScalar b[9];
    matrix.get9(b);
    SkDebugf("{%g, %g, %g},\n{%g, %g, %g},\n{%g, %g, %g}\n", b[0], b[1], b[2],
             b[3], b[4], b[5], b[6], b[7], b[8]);
}
}  // END FIDDLE
