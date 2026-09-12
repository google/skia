// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <array>

REG_FIDDLE(Matrix_SetAffineIdentity, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkScalar affine[6];
    SkMatrix::SetAffineIdentity(affine);
    static constexpr auto names =
            std::to_array<const char*>({"ScaleX", "SkewY", "SkewX", "ScaleY", "TransX", "TransY"});
    for (int i = 0; i < 6; ++i) {
        SkDebugf("%s: %g ", names[i], affine[i]);
    }
    SkDebugf("\n");
}
}  // END FIDDLE
