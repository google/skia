// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9b29ea460d69b4d47323fd9e3e17721e
REG_FIDDLE(Rect_set_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect1 = {3, 4, 1, 2};
    SkDebugf("rect1: {%g, %g, %g, %g}\n", rect1.fLeft, rect1.fTop, rect1.fRight, rect1.fBottom);
    SkRect rect2;
    rect2.set(3, 4, 1, 2);
    SkDebugf("rect2: {%g, %g, %g, %g}\n", rect2.fLeft, rect2.fTop, rect2.fRight, rect2.fBottom);
}
}  // END FIDDLE
