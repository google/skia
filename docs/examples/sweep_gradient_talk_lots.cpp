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
    SkColor4f colors[100];
    for (auto& color : colors) {
        color = SkColor4f::FromColor(randOpaqueColor());
    }
    float cx = 128, cy = 128;
    SkPaint paint;
    paint.setShader(SkShaders::SweepGradient({cx, cy}, {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
