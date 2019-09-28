// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f5b6d371c4d65e5b5ac6eebdd4b237d8
REG_FIDDLE(Matrix_setAffine, 256, 256, true, 0) {
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
        matrix.reset();
        matrix.setAffine(affine);
        matrix.dump();
    }
}
}  // END FIDDLE
