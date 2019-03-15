// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=861f873ba660af8c8bf8b0b83d829cf4
REG_FIDDLE(Rect_044, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 14, 50, 73 };
    rect.outset(5, 13);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
