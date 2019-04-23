// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e4288fabf24ee60b645e8bb6ea0afadf
REG_FIDDLE(Paint_setFilterQuality, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setFilterQuality(kHigh_SkFilterQuality);
    SkDebugf("kHigh_SkFilterQuality %c= paint.getFilterQuality()\n",
            kHigh_SkFilterQuality == paint.getFilterQuality() ? '=' : '!');
}
}  // END FIDDLE
