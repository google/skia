// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2e2acc21d7774df7e0940a30ad2ca99e
REG_FIDDLE(Canvas_043, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkFont font(nullptr, 128);
    canvas->translate(30, 130);
    canvas->save();
    canvas->skew(-.5, 0);
    canvas->drawString("A1", 0, 0, font, paint);
    canvas->restore();
    canvas->save();
    canvas->skew(0, .5);
    paint.setColor(SK_ColorRED);
    canvas->drawString("A1", 0, 0, font, paint);
    canvas->restore();
    canvas->skew(-.5, .5);
    paint.setColor(SK_ColorBLUE);
    canvas->drawString("A1", 0, 0, font, paint);
}
}  // END FIDDLE
