// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a10ad8d97062bc3f40942f47e5108917
REG_FIDDLE(Rect_set, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect i_rect = {3, 4, 1, 2};
    SkDebugf("i_rect: {%d, %d, %d, %d}\n", i_rect.fLeft, i_rect.fTop, i_rect.fRight, i_rect.fBottom);
    SkRect f_rect;
    f_rect.set(i_rect);
    SkDebugf("f_rect: {%g, %g, %g, %g}\n", f_rect.fLeft, f_rect.fTop, f_rect.fRight, f_rect.fBottom);
}
}  // END FIDDLE
