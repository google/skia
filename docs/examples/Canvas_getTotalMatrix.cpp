// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c0d5fa544759704768f47cac91ae3832
REG_FIDDLE(Canvas_getTotalMatrix, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("isIdentity %s\n", canvas->getTotalMatrix().isIdentity() ? "true" : "false");
}
}  // END FIDDLE
