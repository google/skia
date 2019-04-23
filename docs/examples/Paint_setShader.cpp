// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=77e64d5bae9b1ba037fd99252bb4aa58
REG_FIDDLE(Paint_setShader, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setShader(SkShaders::Color(SK_ColorRED));
    canvas->drawRect(SkRect::MakeWH(40, 40), paint);
    paint.setShader(nullptr);
    canvas->translate(50, 0);
    canvas->drawRect(SkRect::MakeWH(40, 40), paint);
}
}  // END FIDDLE
