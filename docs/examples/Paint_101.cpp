#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=aff208b0aab265f273045b27e683c17c
REG_FIDDLE(Paint_101, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(24);
    paint.setTextSkewX(-.25f);
    canvas->drawString("right-leaning", 10, 100, paint);
    paint.setTextSkewX(0);
    canvas->drawString("normal", 10, 60, paint);
    paint.setTextSkewX(.25f);
    canvas->drawString("left-leaning", 10, 20, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
