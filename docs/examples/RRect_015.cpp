// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=44e9a9c2c5ef1af2a616086ff46a9037
REG_FIDDLE(RRect_015, 256, 80, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRect({30, 10, 100, 60});
    canvas->drawRRect(rrect, paint);
    rrect.setEmpty();
    paint.setColor(SK_ColorBLUE);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
