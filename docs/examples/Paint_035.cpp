// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d4ca1f23809b6835c4ba46ea98a86900
REG_FIDDLE(Paint_035, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("kNone_SkFilterQuality %c= paint.getFilterQuality()\n",
            kNone_SkFilterQuality == paint.getFilterQuality() ? '=' : '!');
}
}  // END FIDDLE
