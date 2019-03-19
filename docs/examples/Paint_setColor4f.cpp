// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fa60859e3d03bdc117a05b32e093a8f1
REG_FIDDLE(Paint_setColor4f, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint green1, green2;
    green1.setColor4f({0, 1, 0, 1}, nullptr);  // R=0 G=1 B=0 A=1
    green2.setColor(0xFF00FF00); // A=255 R=0 G=255 B=0
    SkDebugf("green1 %c= green2\n", green1 == green2 ? '=' : '!');
}
}  // END FIDDLE
