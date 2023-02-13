// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <random>

REG_FIDDLE(sweep_gradient_talk_lots, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    std::default_random_engine rng;
    const auto randOpaqueColor = [&rng]() -> SkColor {
        return std::uniform_int_distribution<uint32_t>(0, 0xFFFFFF)(rng) | 0xFF000000;
    };
    SkColor colors[100];
    for (auto& color : colors) {
        color = randOpaqueColor();
    }
    float cx = 128, cy = 128;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeSweep(cx, cy, colors, nullptr, 100));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
