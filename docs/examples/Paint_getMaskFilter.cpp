// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_getMaskFilter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= mask filter\n", paint.getMaskFilter() ? '!' : '=');
   paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 3));
   SkDebugf("nullptr %c= mask filter\n", paint.getMaskFilter() ? '!' : '=');
}
}  // END FIDDLE
