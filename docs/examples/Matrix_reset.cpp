// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ca94f7922bc37ef03bbc51ad70536fcf
REG_FIDDLE(Matrix_reset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix m;
    m.reset();
    SkDebugf("m.isIdentity(): %s\n", m.isIdentity() ? "true" : "false");
}
}  // END FIDDLE
