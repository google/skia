// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ab2c1a55016c8de9172b77fdf69e00a2
REG_FIDDLE(Rect_003, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkSize size = {25.5f, 35.5f};
    SkRect rect = SkRect::MakeSize(size);
    SkDebugf("rect width: %g  height: %g\n", rect.width(), rect.height());
    SkISize floor = size.toFloor();
    rect = SkRect::MakeSize(SkSize::Make(floor));
    SkDebugf("floor width: %g  height: %g\n", rect.width(), rect.height());
}
}  // END FIDDLE
