// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=005f2b207e078baac596681924fe591e
REG_FIDDLE(Canvas_getSaveCount, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas simple;
    SkDebugf("depth = %d\n", simple.getSaveCount());
    simple.save();
    SkDebugf("depth = %d\n", simple.getSaveCount());
    simple.restore();
    SkDebugf("depth = %d\n", simple.getSaveCount());
}
}  // END FIDDLE
