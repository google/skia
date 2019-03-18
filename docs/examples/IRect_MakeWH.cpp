// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e36827a1a6ae2b1c26e7a8a08f325a07
REG_FIDDLE(IRect_MakeWH, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect1 = SkIRect::MakeWH(25, 35);
    SkIRect rect2 = SkIRect::MakeSize({25, 35});
    SkIRect rect3 = SkIRect::MakeXYWH(0, 0, 25, 35);
    SkIRect rect4 = SkIRect::MakeLTRB(0, 0, 25, 35);
    SkDebugf("all %s" "equal\n", rect1 == rect2 && rect2 == rect3 && rect3 == rect4 ?
             "" : "not ");
}
}  // END FIDDLE
