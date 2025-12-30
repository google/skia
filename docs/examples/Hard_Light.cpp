// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Hard_Light, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    const SkColor4f colors[] = { {1,1,1,1}, {0,0,0,0} };
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kHardLight);
    paint.setShader(SkShaders::RadialGradient({ 128, 128}, 100,
                                              {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->clipRect({0, 128, 256, 256});
    canvas->drawPaint(paint);
}
}  // END FIDDLE
