// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Rect_roundOut, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 40.5f, 60.5f };
    SkIRect round;
    rect.roundOut(&round);
    SkDebugf("round: %d, %d, %d, %d\n", round.fLeft, round.fTop, round.fRight, round.fBottom);
}
}  // END FIDDLE
