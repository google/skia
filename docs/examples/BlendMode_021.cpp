// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ac93f30dff13f8a8bb31398de370863b
REG_FIDDLE(BlendMode_021, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    const SkColor colors[] = { 0xFFFFFFFF, 0x3FFFFFFF };
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSoftLight);
    paint.setShader(SkGradientShader::MakeRadial({ 128, 128}, 100, colors,
         nullptr, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode));
    canvas->drawImage(image, 0, 0);
    canvas->drawCircle(128, 128, 100, paint);
}
}  // END FIDDLE
