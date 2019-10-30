// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=abb337da8fc1891f016c61258681c64c
REG_FIDDLE(Rect_roundIn, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 40.5f, 60.5f };
    SkIRect round;
    rect.roundIn(&round);
    SkDebugf("round: %d, %d, %d, %d\n", round.fLeft, round.fTop, round.fRight, round.fBottom);
}
}  // END FIDDLE
