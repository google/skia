// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=dd801faa1e60a0fe9e0657674461e063
REG_FIDDLE(Rect_Make_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect i_rect1 = {2, 35, 22, 53};
    SkRect f_rect = SkRect::Make(i_rect1);
    f_rect.offset(0.49f, 0.49f);
    SkIRect i_rect2;
    f_rect.round(&i_rect2);
    SkDebugf("i_rect1 %c= i_rect2\n", i_rect1 == i_rect2? '=' : '!');
}
}  // END FIDDLE
