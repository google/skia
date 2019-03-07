// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6e70f18300bd676a3c056ceb6b62f8df
REG_FIDDLE(Paint_040, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint green1, green2;
    unsigned a = 255;
    unsigned r = 0;
    unsigned g = 255;
    unsigned b = 0;
    green1.setColor((a << 24) + (r << 16) + (g << 8) + (b << 0));
    green2.setColor(0xFF00FF00);
    SkDebugf("green1 %c= green2\n", green1 == green2 ? '=' : '!');
}
}  // END FIDDLE
