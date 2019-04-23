// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8b9e5a9af0a9b878f76919534d88f41e
REG_FIDDLE(Rect_round, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 40.5f, 60.5f };
    SkIRect round;
    rect.round(&round);
    SkDebugf("round: %d, %d, %d, %d\n", round.fLeft, round.fTop, round.fRight, round.fBottom);
}
}  // END FIDDLE
