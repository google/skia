// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Xor, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kXor);
    for (auto color : { SkColors::kRed, SkColors::kBlue, SkColors::kGreen } ) {
        SkColor4f colors[] = { color, color.withAlphaByte(192), color.withAlphaByte(128),
                             color.withAlpha(0) };
        paint.setShader(SkShaders::RadialGradient({64, 64}, 100,
                                                  {{colors, {}, SkTileMode::kClamp}, {}}));
        canvas->drawCircle(64, 64, 100, paint);
        canvas->translate(64, 64);
    }
}
}  // END FIDDLE
