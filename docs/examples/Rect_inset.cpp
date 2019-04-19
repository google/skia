// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=dae21340941dc6e4d048816dfd9f204c
REG_FIDDLE(Rect_inset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 14, 50, 73 };
    rect.inset(5, 13);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
