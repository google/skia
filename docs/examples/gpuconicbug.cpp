// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(gpuconicbug, 300, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(SkBits2Float(0x43507c37), SkBits2Float(0x4278037b));  // 208.485f, 62.0034f
    path.lineTo(SkBits2Float(0x434fdd66), SkBits2Float(0x41701eb0));  // 207.865f, 15.0075f
    path.lineTo(SkBits2Float(0x434fdd67), SkBits2Float(0x41701eb0));  // 207.865f, 15.0075f
    path.conicTo(
            SkBits2Float(0x4350260c), SkBits2Float(0x41700f50), SkBits2Float(0x43506eb3),
            SkBits2Float(0x417007a8),
            SkBits2Float(0x3f7fffa5));  // 208.149f, 15.0037f, 208.432f, 15.0019f, 0.999995f
    path.lineTo(SkBits2Float(0x4350be1b), SkBits2Float(0x427800df));  // 208.743f, 62.0009f
    path.lineTo(SkBits2Float(0x4350be1b), SkBits2Float(0x427800de));  // 208.743f, 62.0008f
    path.conicTo(
            SkBits2Float(0x43509d29), SkBits2Float(0x427801bc), SkBits2Float(0x43507c38),
            SkBits2Float(0x42780379),
            SkBits2Float(0x3f7fffa5));  // 208.614f, 62.0017f, 208.485f, 62.0034f, 0.999995f
    path.lineTo(SkBits2Float(0x43507c37), SkBits2Float(0x4278037b));  // 208.485f, 62.0034f
    path.close();

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);
    canvas->scale(.533333333333333f, .5333333333333f);
    paint.setColor(SK_ColorRED);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
