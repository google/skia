// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8512ea2176f36e8f1aeef311ff228790
REG_FIDDLE(Paint_getColor4f, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    SkColor4f y = paint.getColor4f();
    SkDebugf("Yellow is %d%% red, %d%% green, and %d%% blue.\n", (int) (y.fR * 100),
            (int) (y.fG * 100), (int) (y.fB * 100));
}
}  // END FIDDLE
