// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Lighten, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    SkColor4f colors[] = { SkColors::kBlack, SkColors::kWhite };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(horz, {{colors, {}, SkTileMode::kClamp}, {}}));
    paint.setBlendMode(SkBlendMode::kLighten);
    canvas->drawPaint(paint);
}
}  // END FIDDLE
