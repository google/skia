// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=94039c3cc9e911c8ab2993d56fd06210
REG_FIDDLE(IRect_020, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = {3, 4, 1, 2};
    for (int i = 0; i < 2; ++i) {
    SkDebugf("rect: {%d, %d, %d, %d} is %s" "empty\n", rect.fLeft, rect.fTop,
             rect.fRight, rect.fBottom, rect.isEmpty() ? "" : "not ");
    rect.setEmpty();
    }
}
}  // END FIDDLE
