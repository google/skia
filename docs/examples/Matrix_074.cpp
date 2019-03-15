// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e10adbd0bcc940c5d4d872db0e78e892
REG_FIDDLE(Matrix_074, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkScalar affine[6];
    SkMatrix::SetAffineIdentity(affine);
    const char* names[] = { "ScaleX", "SkewY", "SkewX", "ScaleY", "TransX", "TransY" };
    for (int i = 0; i < 6; ++i) {
        SkDebugf("%s: %g ", names[i], affine[i]);
    }
    SkDebugf("\n");
}
}  // END FIDDLE
