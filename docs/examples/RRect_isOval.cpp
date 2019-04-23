#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4dfdb28d8343958425f2c1323fe8170d
REG_FIDDLE(RRect_isOval, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(16);
    SkRRect rrect = SkRRect::MakeRectXY({30, 10, 100, 60}, 40, 30);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isOval() ? "oval" : "not oval", 64, 90, paint);
    rrect.setRectXY(rrect.getBounds(), 35, 25);
    canvas->translate(128, 0);
    canvas->drawRRect(rrect, paint);
    canvas->drawString(rrect.isOval() ? "oval" : "not oval", 64, 90, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
