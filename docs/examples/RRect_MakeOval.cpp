// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0b99ee38fd154f769f6031242e02fa7a
REG_FIDDLE(RRect_MakeOval, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeOval({30, 10, 100, 60});
    canvas->drawRRect(rrect, paint);
    rrect.setRect(rrect.getBounds());
    paint.setColor(SK_ColorBLUE);
    paint.setBlendMode(SkBlendMode::kDifference);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
