// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ef269937ade7e7353635121d9a64f9f7
REG_FIDDLE(Paint_reset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setColor(SK_ColorRED);
    paint1.reset();
    SkDebugf("paint1 %c= paint2", paint1 == paint2 ? '=' : '!');
}
}  // END FIDDLE
