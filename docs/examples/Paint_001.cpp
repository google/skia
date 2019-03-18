// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b99971ad0ef243d617925289d963b62d
REG_FIDDLE(Paint_copy_const_SkPaint, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1;
    paint1.setColor(SK_ColorRED);
    SkPaint paint2(paint1);
    paint2.setColor(SK_ColorBLUE);
    SkDebugf("SK_ColorRED %c= paint1.getColor()\n", SK_ColorRED == paint1.getColor() ? '=' : '!');
    SkDebugf("SK_ColorBLUE %c= paint2.getColor()\n", SK_ColorBLUE == paint2.getColor() ? '=' : '!');
}
}  // END FIDDLE
