// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1db94b2c76e0a7a71856532335fa56b6
REG_FIDDLE(IRect_makeInset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 10, 50, 20, 60 };
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect = rect.makeInset(15, 32);
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
