// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5295b07fe4d2cdcd077979a9e19854d9
REG_FIDDLE(RRect_018, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRRect rrect = SkRRect::MakeRect({30, 10, 100, 60});
    canvas->drawRRect(rrect, paint);
    rrect.setOval(rrect.getBounds());
    paint.setColor(SK_ColorBLUE);
    canvas->drawRRect(rrect, paint);
}
}  // END FIDDLE
