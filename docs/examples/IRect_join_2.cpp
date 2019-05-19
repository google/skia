// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=75fd81c1d3512e63890d085593018876
REG_FIDDLE(IRect_join_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 10, 20, 15, 25};
    rect.join({50, 60, 55, 65});
    SkDebugf("join: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
