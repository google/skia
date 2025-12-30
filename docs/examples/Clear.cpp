// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Clear, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->saveLayer(nullptr, nullptr);
    canvas->drawColor(SK_ColorYELLOW, SkBlendMode::kClear);
    SkPaint paint;
    for (auto color : { SkColors::kRed, SkColors::kBlue, SkColors::kGreen } ) {
        SkColor4f colors[] = { color, color.withAlpha(0) };
        paint.setShader(SkShaders::RadialGradient({ 64, 64}, 100,
                {{colors, {}, SkTileMode::kClamp}, {}}));
        canvas->drawCircle(64, 64, 100, paint);
        canvas->translate(64, 64);
    }
    canvas->restore();
}
}  // END FIDDLE
