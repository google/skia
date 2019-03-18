// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3979c865bb482e6ef1fafc71e56bbb91
REG_FIDDLE(Matrix_setIdentity, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix m;
    m.setIdentity();
    SkDebugf("m.isIdentity(): %s\n", m.isIdentity() ? "true" : "false");
}
}  // END FIDDLE
