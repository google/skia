// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ce5319c036c9b5086da8a0009fe409f8
REG_FIDDLE(Matrix_rectStaysRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    for (SkScalar angle: { 0, 90, 180, 270 } ) {
        matrix.setRotate(angle);
        SkDebugf("rectStaysRect: %s\n", matrix.rectStaysRect() ? "true" : "false");
    }
}
}  // END FIDDLE
