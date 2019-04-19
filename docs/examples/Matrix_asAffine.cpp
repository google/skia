// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3325bdf82bd86d9fbc4199f248afa82c
REG_FIDDLE(Matrix_asAffine, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setAll(2, 3, 4, 5, 6, 7, 0, 0, 1);
    SkScalar affine[6];
    if (matrix.asAffine(affine)) {
        const char* names[] = { "ScaleX", "SkewY", "SkewX", "ScaleY", "TransX", "TransY" };
        for (int i = 0; i < 6; ++i) {
            SkDebugf("%s: %g ", names[i], affine[i]);
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
