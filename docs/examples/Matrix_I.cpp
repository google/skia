// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d961d91020f19037204a8c3fd8cb1060
REG_FIDDLE(Matrix_I, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix m1, m2, m3;
    m1.reset();
    m2.setIdentity();
    m3 = SkMatrix::I();
    SkDebugf("m1 %c= m2\n", m1 == m2 ? '=' : '!');
    SkDebugf("m2 %c= m3\n", m1 == m2 ? '=' : '!');
}
}  // END FIDDLE
