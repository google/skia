// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ab57b232acef69f26de9cb23d23c8a1a
REG_FIDDLE(Matrix_isFixedStepInX, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    for (SkScalar px : { 0.0f, 0.1f } ) {
        for (SkScalar py : { 0.0f, 0.1f } ) {
            for (SkScalar sy : { 1, 2 } ) {
                matrix.setAll(1, 0, 0,   0, sy, 0,   px, py, 1);
                matrix.dump();
                SkDebugf("isFixedStepInX: %s\n", matrix.isFixedStepInX() ? "true" : "false");
            }
        }
    }
}
}  // END FIDDLE
