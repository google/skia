#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1a7a5062725139760962582f599f1b97
REG_FIDDLE(Typeface_Methods, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTypeface(SkTypeface::MakeFromName(nullptr, SkFontStyle()));
    paint.setAntiAlias(true);
    paint.setTextSize(36);
    canvas->drawString("A Big Hello!", 10, 40, paint);
    paint.setTypeface(nullptr);
    paint.setFakeBoldText(true);
    canvas->drawString("A Big Hello!", 10, 80, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
