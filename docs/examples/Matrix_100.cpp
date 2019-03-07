// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=13adba0ecf5f82247cf051b4fa4d8a9c
REG_FIDDLE(Matrix_100, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setAll(1, 0, 0,  0, 1, 0,   0, 0, 0);
    if (matrix.invert(&matrix)) {
        SkScalar factor[2] = {2, 2};
        bool result = matrix.getMinMaxScales(factor);
        SkDebugf("matrix.getMinMaxScales() %s %g %g\n",
                result ? "true" : "false", factor[0], factor[1]);
    }
}
}  // END FIDDLE
