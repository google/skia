// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(sweep_gradient_talk_lots, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRandom rand;
    SkColor colors[100];
    for (auto& color : colors) {
        color = rand.nextU() | (0xFF << 24);
    }
    float cx = 128, cy = 128;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeSweep(cx, cy, colors, nullptr, 100));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
