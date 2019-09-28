// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1d6f67904c88a806c3731879e9af4ae5
REG_FIDDLE(Matrix_getMinScale, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setScale(42, 24);
    SkDebugf("matrix.getMinScale() %g\n", matrix.getMinScale());
}
}  // END FIDDLE
