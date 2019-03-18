// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=04eb33f0fd376f2942ca5f1c7f6cbcfc
REG_FIDDLE(Rect_offset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 14, 50, 73 };
    rect.offset(5, 13);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
