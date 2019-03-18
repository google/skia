// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2b24a1247637cbc94f8b3c77d37ed3e2
REG_FIDDLE(RRect_MakeRectXY, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRectXY({30, 10, 100, 60}, 20, 20);
    canvas->drawRRect(rrect, paint);
    rrect.setRect(rrect.getBounds());
    paint.setColor(SK_ColorBLUE);
    paint.setBlendMode(SkBlendMode::kModulate);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
