// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Rect_058, 256, 256, true, 0) {
// HASH=8b9e5a9af0a9b878f76919534d88f41e
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 40.5f, 60.5f };
    SkIRect round;
    rect.round(&round);
    SkDebugf("round: %d, %d, %d, %d\n", round.fLeft, round.fTop, round.fRight, round.fBottom);
}
}
