// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7481a948e34672720337a631830586dd
REG_FIDDLE(Paint_equal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setColor(SK_ColorRED);
    paint2.setColor(0xFFFF0000);
    SkDebugf("paint1 %c= paint2\n", paint1 == paint2 ? '=' : '!');
    float intervals[] = { 5, 5 };
    paint1.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));
    paint2.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));
    SkDebugf("paint1 %c= paint2\n", paint1 == paint2 ? '=' : '!');
}
}  // END FIDDLE
