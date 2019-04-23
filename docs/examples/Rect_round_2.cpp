// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ef7ae1dd522c235b0afe41b55a624f46
REG_FIDDLE(Rect_round_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 40.5f, 60.5f };
    SkIRect round = rect.round();
    SkDebugf("round: %d, %d, %d, %d\n", round.fLeft, round.fTop, round.fRight, round.fBottom);
}
}  // END FIDDLE
