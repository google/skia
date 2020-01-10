// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(flag_us_1960, 486, 256, false, 0) {
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
    SkParsePath::FromSVGString(
            "M 0 -120 L 70.5342 97.0819 L -114.127 -37.082 L 114.127 -37.0821 L -70.5342 "
            "97.0819 Z",
            &star);
    for (int x = 1; x < 12; ++x) {
        for (int y = 1; y < 10; ++y) {
            if ((x + y) % 2 == 0) {
                SkAutoCanvasRestore autoCanvasRestore(canvas, true);
                canvas->translate(x * 247, y * 210);
                canvas->drawPath(star, paint);
            }
        }
    }
}
}  // END FIDDLE
