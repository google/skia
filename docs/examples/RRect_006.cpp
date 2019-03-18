#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=099d79ecfbdfb0a19c10deb7201859c3
REG_FIDDLE(RRect_isEmpty, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(16);
    SkRRect rrect = SkRRect::MakeRectXY({30, 10, 100, 60}, 10, 5);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isEmpty() ? "empty" : "not empty", 64, 90, paint);
    rrect.inset(40, 0);
    canvas->translate(128, 0);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isEmpty() ? "empty" : "not empty", 64, 90, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
