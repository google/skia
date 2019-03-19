// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=faa660ac19eaddc3f3eab57a0bddfdcb
REG_FIDDLE(Rect_MakeIWH, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect i_rect = SkIRect::MakeWH(25, 35);
    SkRect  f_rect = SkRect::MakeIWH(25, 35);
    SkDebugf("i_rect width: %d f_rect width:%g\n", i_rect.width(), f_rect.width());
    i_rect = SkIRect::MakeWH(125000111, 0);
    f_rect = SkRect::MakeIWH(125000111, 0);
    SkDebugf("i_rect width: %d f_rect width:%.0f\n", i_rect.width(), f_rect.width());
}
}  // END FIDDLE
