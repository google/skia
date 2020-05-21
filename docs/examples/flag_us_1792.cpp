// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(flag_us_1792, 486, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    canvas->clear(SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF));
    canvas->scale(256.0f / 3900, 256.0f / 3900);
    paint.setColor(SkColorSetARGB(0xFF, 0xB2, 0x22, 0x34));
    for (int s = 0; s < 13; s += 2) {
        canvas->drawIRect({0, s * 300, 7410, (s + 1) * 300}, paint);
    }
    paint.setColor(SkColorSetARGB(0xFF, 0x3C, 0x3B, 0x6E));
    canvas->drawIRect({0, 0, 2964, 2100}, paint);
    paint.setColor(SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF));
    paint.setAntiAlias(true);
    SkPath star;
    SkParsePath::FromSVGString("M 0 -150 L 88 121 L -143 -46 L 143 -46 L -88 121 Z", &star);
    for (int i = 0; i < 13; ++i) {
        SkMatrix matrix = SkMatrix::Translate(1482, 1050);
        matrix.preRotate((360.0 / 13) * i);
        matrix.preTranslate(0, -785);
        SkAutoCanvasRestore autoCanvasRestore(canvas, true);
        canvas->concat(matrix);
        canvas->drawPath(star, paint);
    }
}
}  // END FIDDLE
