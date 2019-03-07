// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8dc91284493dd012cca3d0ce4c66bda4
REG_FIDDLE(IRect_032, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 8, 11, 19, 22 };
    rect.adjust(2, -1, 1, -2);
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
