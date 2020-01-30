// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(UnicornPoop, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    for (int r = 0; r <= 255; r += 10) {
        for (int g = 0; g <= 255; g += 10) {
            for (int b = 0; b <= 255; b += 10) {
                p.setColor(SkColorSetRGB(r, g, b));
                canvas->drawRect(SkRect::MakeXYWH(r, g, b, 1), p);
            }
        }
    }
}
}  // END FIDDLE
