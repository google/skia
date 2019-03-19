// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7420bf0a7cae5c6577c4c4a4613e7e7e
REG_FIDDLE(RGBA4f_vec_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f color = SkColor4f::FromColor(0x884488CC);
    SkDebugf("red=%g green=%g blue=%g alpha=%g\n", color.fR, color.fG, color.fB, color.fA);
    float* array = color.vec();
    array[3] = 1;
    SkDebugf("[0]=%g [1]=%g [2]=%g [3]=%g\n", array[0], array[1], array[2], array[3]);
}
}  // END FIDDLE
