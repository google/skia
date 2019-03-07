// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bedb04b7b3e1af3e8039f9cffe66989e
REG_FIDDLE(Rect_042, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 14, 50, 73 };
    rect.offsetTo(15, 27);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
