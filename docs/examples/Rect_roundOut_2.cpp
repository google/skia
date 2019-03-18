// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e09a6a12869a8ac21e9c2af98a5bb686
REG_FIDDLE(Rect_roundOut_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 40.5f, 60.5f };
    SkRect round;
    rect.roundOut(&round);
    SkDebugf("round: %g, %g, %g, %g\n", round.fLeft, round.fTop, round.fRight, round.fBottom);
}
}  // END FIDDLE
