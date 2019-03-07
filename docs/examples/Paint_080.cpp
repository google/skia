// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5ac4b31371726da87bb7390b385e9fee
REG_FIDDLE(Paint_080, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= mask filter\n", paint.getMaskFilter() ? '!' : '=');
   paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 3));
   SkDebugf("nullptr %c= mask filter\n", paint.getMaskFilter() ? '!' : '=');
}
}  // END FIDDLE
