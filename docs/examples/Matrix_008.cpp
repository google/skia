// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=73ac71a8a30841873577c11c6c9b38ee
REG_FIDDLE(Matrix_isTranslate, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    for (SkScalar scaleX : { 1, 2 } ) {
        for (SkScalar translateX : { 0, 20 } ) {
            matrix.setAll(scaleX, 0, translateX,   0, 1, 0,    0, 0, 1);
            SkDebugf("is translate: %s\n", matrix.isTranslate() ? "true" : "false");
        }
    }
}
}  // END FIDDLE
