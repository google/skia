#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6510c9e2f57b83c47e67829e7a68d493
REG_FIDDLE(Paint_setTextSize, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("12 %c= text size\n", 12 == paint.getTextSize() ? '=' : '!');
    paint.setTextSize(-20);
    SkDebugf("12 %c= text size\n", 12 == paint.getTextSize() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
