// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4577e2dcb086b241bb43d8b89ee0b0dd
REG_FIDDLE(RRect_028, 256, 120, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRectXY({20, 20, 220, 100}, 15, 15);
    canvas->drawRRect(rrect, paint);
    paint.setColor(SK_ColorWHITE);
    rrect = SkRRect::MakeOval(rrect.getBounds());
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
