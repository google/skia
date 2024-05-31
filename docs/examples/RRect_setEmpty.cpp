// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(RRect_setEmpty, 256, 80, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRect({30, 10, 100, 60});
    canvas->drawRRect(rrect, paint);
    rrect.setEmpty();
    paint.setColor(SK_ColorBLUE);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
