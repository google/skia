// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ead6bdcf2ae77ec19a1c5a96f5b31af8
REG_FIDDLE(IRect_setLTRB, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect1 = {3, 4, 1, 2};
    SkDebugf("rect1: {%d, %d, %d, %d}\n", rect1.fLeft, rect1.fTop, rect1.fRight, rect1.fBottom);
    SkIRect rect2;
    rect2.setLTRB(3, 4, 1, 2);
    SkDebugf("rect2: {%d, %d, %d, %d}\n", rect2.fLeft, rect2.fTop, rect2.fRight, rect2.fBottom);
}
}  // END FIDDLE
