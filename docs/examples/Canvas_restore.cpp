// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e78471212a67f2f4fd39496e17a30d17
REG_FIDDLE(Canvas_restore, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas simple;
    SkDebugf("depth = %d\n", simple.getSaveCount());
    simple.restore();
    SkDebugf("depth = %d\n", simple.getSaveCount());
}
}  // END FIDDLE
