// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=31a4c575499e76def651eb65994876f0
REG_FIDDLE(IRect_028, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 10, 14, 50, 73 };
    rect.offset({5, 13});
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
