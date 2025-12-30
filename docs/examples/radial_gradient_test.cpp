// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(radial_gradient_test, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
#define SIZE 121
    SkScalar half = SIZE * 0.5f;
    const SkColor4f preColor = {1,0,0,1};   // clamp color before start
    const SkColor4f postColor = {0,0,1,1};  // clamp color after end
    const SkColor4f color0 = {0,0,0,1};
    const SkColor4f color1 = {0,1,0,1};

    SkColor4f cs[] = {preColor, color0, color1, postColor};
    SkScalar pos[] = {0, 0, 1, 1};

    auto s = SkShaders::RadialGradient({half, half}, half - 10,
                                       {{cs, pos, SkTileMode::kClamp}, {}});

    SkPaint p;
    const SkRect rect = SkRect::MakeWH(SIZE, SIZE);
    p.setShader(s);
    canvas->drawRect(rect, p);
}
}  // END FIDDLE
