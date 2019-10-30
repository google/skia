// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=72d41f890203109a41f589a7403acae9
REG_FIDDLE(Paint_getColor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    SkColor y = paint.getColor();
    SkDebugf("Yellow is %d%% red, %d%% green, and %d%% blue.\n", (int) (SkColorGetR(y) / 2.55f),
            (int) (SkColorGetG(y) / 2.55f), (int) (SkColorGetB(y) / 2.55f));
}
}  // END FIDDLE
