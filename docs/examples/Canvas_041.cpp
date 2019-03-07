// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=963789ac8498d4e505748ab3b15cdaa5
REG_FIDDLE(Canvas_041, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->translate(128, 128);
    canvas->drawCircle(0, 0, 60, paint);
    canvas->save();
    canvas->rotate(10 * 360 / 60);   // 10 minutes of 60 scaled to 360 degrees
    canvas->drawLine(0, 0, 0, -50, paint);
    canvas->restore();
    canvas->rotate((5 + 10.f/60) * 360 / 12); // 5 and 10/60 hours of 12 scaled to 360 degrees
    canvas->drawLine(0, 0, 0, -30, paint);
}
}  // END FIDDLE
