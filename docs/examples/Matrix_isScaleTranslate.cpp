// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6287e29674a487eb94174992d45b9a34
REG_FIDDLE(Matrix_isScaleTranslate, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    for (SkScalar scaleX : { 1, 2 } ) {
        for (SkScalar translateX : { 0, 20 } ) {
            matrix.setAll(scaleX, 0, translateX,   0, 1, 0,    0, 0, 1);
            SkDebugf("is scale-translate: %s\n", matrix.isScaleTranslate() ? "true" : "false");
        }
    }
}
}  // END FIDDLE
