// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Soft_Light, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    const SkColor4f colors[] = { {1,1,1,1}, {1,1,1,0x3F/255.f} };
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSoftLight);
    paint.setShader(SkShaders::RadialGradient({ 128, 128}, 100,
                                              {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawImage(image, 0, 0);
    canvas->drawCircle(128, 128, 100, paint);
}
}  // END FIDDLE
