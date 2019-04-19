// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=af0b72360c1c7a25b4754bfa47011dd5
REG_FIDDLE(Matrix_InvalidMatrix, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("scaleX %g\n", SkMatrix::InvalidMatrix().getScaleX());
}
}  // END FIDDLE
