// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cb62e4755789ed32f7120dc55984959d
REG_FIDDLE(Paint_setARGB, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint transRed1, transRed2;
    transRed1.setARGB(255 / 2, 255, 0, 0);
    transRed2.setColor(SkColorSetARGB(255 / 2, 255, 0, 0));
    SkDebugf("transRed1 %c= transRed2", transRed1 == transRed2 ? '=' : '!');
}
}  // END FIDDLE
