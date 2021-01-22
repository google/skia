// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(persp_text_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkMatrix persp;
    persp.setAll(0.9839f, 0, 0, 0.2246f, 0.6829f, 0, 0.0002352f, -0.0003844f, 1);
    canvas->concat(persp);
    canvas->translate(100, 0);
    canvas->scale(375, 375);

    const char text[] = "SKIA";
    canvas->drawString(text, 0, 0, SkFont(nullptr, 0.1f), paint);
}
}  // END FIDDLE
