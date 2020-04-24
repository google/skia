// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6ddc0360512dfb9947e75c17e6a8103d
REG_FIDDLE(Paint_setAlpha, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(0x00112233);
    paint.setAlpha(0x44);
    SkDebugf("0x44112233 %c= paint.getColor()\n", 0x44112233 == paint.getColor() ? '=' : '!');
}
}  // END FIDDLE
