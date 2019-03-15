// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a2734ff23b35653956a3002e5c29ff91
REG_FIDDLE(IRect_029, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 10, 14, 50, 73 };
    rect.offsetTo(15, 27);
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
