// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=85442cf8d0bce6b5a777853bc36a4dc4
REG_FIDDLE(Canvas_drawString, 256, 48, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkFont font;
    canvas->drawString("a small hello", 20, 20, font, paint);
}
}  // END FIDDLE
