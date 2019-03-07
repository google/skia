// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8009d30f431e01f8aea4808e9017d9bf
REG_FIDDLE(Rect_001, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect1 = SkRect::MakeWH(25, 35);
    SkRect rect2 = SkRect::MakeIWH(25, 35);
    SkRect rect3 = SkRect::MakeXYWH(0, 0, 25, 35);
    SkRect rect4 = SkRect::MakeLTRB(0, 0, 25, 35);
    SkDebugf("all %s" "equal\n", rect1 == rect2 && rect2 == rect3 && rect3 == rect4 ?
             "" : "not ");
}
}  // END FIDDLE
