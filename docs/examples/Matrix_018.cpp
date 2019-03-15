// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=708b1a548a2f8661b2ab570782fbc751
REG_FIDDLE(Matrix_018, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setScale(42, 24);
    SkDebugf("matrix.getScaleY() %c= 24\n", matrix.getScaleY() == 24 ? '=' : '!');
}
}  // END FIDDLE
