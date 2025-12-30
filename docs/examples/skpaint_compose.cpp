// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_compose, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f colors[2] = {SkColors::kBlue, SkColors::kYellow};
    SkPaint paint;
    paint.setShader(SkShaders::Blend(SkBlendMode::kDifference,
                                     SkShaders::RadialGradient({128.0f, 128.0f}, 180.0f,
                                                            {{colors, {}, SkTileMode::kClamp}, {}}),
                                     SkShaders::MakeTurbulence(0.025f, 0.025f, 2, 0.0f, nullptr)));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
