// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Shader_Methods_a, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkPoint center = { 50, 50 };
   SkScalar radius = 50;
   const SkColor colors[] = { 0xFFFFFFFF, 0xFF000000 };
   paint.setShader(SkGradientShader::MakeRadial(center, radius, colors,
        nullptr, std::size(colors), SkTileMode::kClamp));
   for (SkScalar a : { 0.3f, 0.6f, 1.0f } ) {
       paint.setAlpha((int) (a * 255));
       canvas->drawCircle(center.fX, center.fY, radius, paint);
       canvas->translate(70, 70);
   }
}
}  // END FIDDLE
