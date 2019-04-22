// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5abde56ca2f89a18b8e231abd1b57c56
REG_FIDDLE(Color_Filter_Methods, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColorFilter(SkColorMatrixFilter::MakeLightingFilter(0xFFFFFF, 0xFF0000));
    for (SkColor c : { SK_ColorBLACK, SK_ColorGREEN } ) {
        paint.setColor(c);
        canvas->drawRect(SkRect::MakeXYWH(10, 10, 50, 50), paint);
        paint.setAlpha(0x80);
        canvas->drawRect(SkRect::MakeXYWH(60, 60, 50, 50), paint);
        canvas->translate(100, 0);
    }
}
}  // END FIDDLE
