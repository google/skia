// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a0f166715d6479f91258d854e63e586d
REG_FIDDLE(Path_116, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkDebugf("empty genID = %u\n", path.getGenerationID());
    path.lineTo(1, 2);
    SkDebugf("1st lineTo genID = %u\n", path.getGenerationID());
    path.rewind();
    SkDebugf("empty genID = %u\n", path.getGenerationID());
    path.lineTo(1, 2);
    SkDebugf("2nd lineTo genID = %u\n", path.getGenerationID());
}
}  // END FIDDLE
