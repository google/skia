// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a9b56a26ca469bab9ab10e16f62fb2e2
REG_FIDDLE(Clear, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->saveLayer(nullptr, nullptr);
    canvas->drawColor(SK_ColorYELLOW, SkBlendMode::kClear);
    SkPaint paint;
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {
        SkColor colors[] = { color, SkColorSetA(color, 0) };
        paint.setShader(SkGradientShader::MakeRadial({ 64, 64}, 100,
                colors, nullptr, SK_ARRAY_COUNT(colors), SkTileMode::kClamp));
        canvas->drawCircle(64, 64, 100, paint);
        canvas->translate(64, 64);
    }
    canvas->restore();
}
}  // END FIDDLE
