// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2cf67542d45ef5d7a7efb673b651ff54
REG_FIDDLE(Rect_setEmpty, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = {3, 4, 1, 2};
    for (int i = 0; i < 2; ++i) {
    SkDebugf("rect: {%g, %g, %g, %g} is %s" "empty\n", rect.fLeft, rect.fTop,
             rect.fRight, rect.fBottom, rect.isEmpty() ? "" : "not ");
    rect.setEmpty();
    }
}
}  // END FIDDLE
