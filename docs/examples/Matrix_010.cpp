// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7a234c96608fb7cb8135b9940b0b15f7
REG_FIDDLE(Matrix_010, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    for (SkScalar angle: { 0, 90, 180, 270 } ) {
        matrix.setRotate(angle);
        SkDebugf("preservesAxisAlignment: %s\n", matrix.preservesAxisAlignment() ? "true" : "false");
    }
}
}  // END FIDDLE
