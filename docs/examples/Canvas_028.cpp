// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9ed0d56436e114c7097fd49eed1aea47
REG_FIDDLE(Canvas_028, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("depth = %d\n", canvas->getSaveCount());
    canvas->save();
    canvas->save();
    SkDebugf("depth = %d\n", canvas->getSaveCount());
    canvas->restoreToCount(0);
    SkDebugf("depth = %d\n", canvas->getSaveCount());
}
}  // END FIDDLE
