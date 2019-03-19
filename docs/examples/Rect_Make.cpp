// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e866f5e4f6ac52e89acadf48e54ac8e0
REG_FIDDLE(Rect_Make, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect1 = SkRect::MakeSize({2, 35});
    SkRect rect2 = SkRect::MakeIWH(2, 35);
    SkDebugf("rect1 %c= rect2\n", rect1 == rect2 ? '=' : '!');
}
}  // END FIDDLE
