#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8cc1f21c98c0416f7724ad218f557a00
REG_FIDDLE(RRect_038, 256, 110, false, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({10, 10, 110, 80});
    SkRRect corrupt = rrect;
    *((float*) &corrupt) = 120;
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString(rrect.isValid() ? "is valid" : "is corrupted", 55, 100, paint);
    canvas->drawString(corrupt.isValid() ? "is valid" : "is corrupted", 185, 100, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRRect(rrect, paint);
    canvas->translate(120, 0);
    canvas->drawRRect(corrupt, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
