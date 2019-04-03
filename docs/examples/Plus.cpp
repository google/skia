// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=05383441e510d54008402e128fc8ad2b
REG_FIDDLE(Plus, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorBLACK);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kPlus);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {
        SkColor colors[] = { color, SkColorSetA(color, 192), SkColorSetA(color, 128),
                             SkColorSetA(color, 0) };
        paint.setShader(SkGradientShader::MakeRadial({ 64, 64}, 100,
                colors, nullptr, SK_ARRAY_COUNT(colors), SkTileMode::kClamp));
        canvas->drawCircle(64, 64, 100, paint);
        canvas->translate(64, 64);
    }
}
}  // END FIDDLE
