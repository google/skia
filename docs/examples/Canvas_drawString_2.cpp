// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=435178c09feb3bfec5e35d983609a013
REG_FIDDLE(Canvas_drawString_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkString string("a small hello");
    canvas->drawString(string, 20, 20, SkFont(), paint);
}
}  // END FIDDLE
