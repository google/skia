// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(radial_gradient_test, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
#define SIZE 121
    SkScalar half = SIZE * 0.5f;
    const SkColor preColor = 0xFFFF0000;   // clamp color before start
    const SkColor postColor = 0xFF0000FF;  // clamp color after end
    const SkColor color0 = 0xFF000000;
    const SkColor color1 = 0xFF00FF00;

    SkColor cs[] = {preColor, color0, color1, postColor};
    SkScalar pos[] = {0, 0, 1, 1};

    auto s = SkGradientShader::MakeRadial({half, half}, half - 10, cs, pos, 4,
                                          SkTileMode::kClamp);

    SkPaint p;
    const SkRect rect = SkRect::MakeWH(SIZE, SIZE);
    p.setShader(s);
    canvas->drawRect(rect, p);
}
}  // END FIDDLE
