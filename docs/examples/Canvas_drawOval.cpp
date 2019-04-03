// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8b6b86f8a022811cd29a9c6ab771df12
REG_FIDDLE(Canvas_drawOval, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(0xFF3f5f9f);
    SkColor  kColor1 = SkColorSetARGB(0xff, 0xff, 0x7f, 0);
    SkColor  g1Colors[] = { kColor1, SkColorSetA(kColor1, 0x20) };
    SkPoint  g1Points[] = { { 0, 0 }, { 0, 100 } };
    SkScalar pos[] = { 0.2f, 1.0f };
    SkRect bounds = SkRect::MakeWH(80, 70);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(SkGradientShader::MakeLinear(g1Points, g1Colors, pos, SK_ARRAY_COUNT(g1Colors),
            SkTileMode::kClamp));
    canvas->drawOval(bounds , paint);
}
}  // END FIDDLE
