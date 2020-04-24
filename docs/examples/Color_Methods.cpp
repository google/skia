// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=214b559d75c65a7bef6ef4be1f860053
REG_FIDDLE(Color_Methods, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(0x8000FF00);  // transparent green
    canvas->drawCircle(50, 50, 40, paint);
    paint.setARGB(128, 255, 0, 0); // transparent red
    canvas->drawCircle(80, 50, 40, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setAlpha(0x80);
    canvas->drawCircle(65, 65, 40, paint);
}
}  // END FIDDLE
