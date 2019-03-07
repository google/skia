// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Matrix_039, 256, 256, true, 0) {
// HASH=3979c865bb482e6ef1fafc71e56bbb91
void draw(SkCanvas* canvas) {
    SkMatrix m;
    m.setIdentity();
    SkDebugf("m.isIdentity(): %s\n", m.isIdentity() ? "true" : "false");
}
}
