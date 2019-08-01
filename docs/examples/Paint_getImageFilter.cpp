// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c11f8eaa1dd149bc18db21e23ce26904
REG_FIDDLE(Paint_getImageFilter, 256, 256, true, 0) {
#include "include/effects/SkImageFilters.h"

void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= image filter\n", paint.getImageFilter() ? '!' : '=');
   paint.setImageFilter(SkImageFilters::Blur(3, 3, nullptr, nullptr));
   SkDebugf("nullptr %c= image filter\n", paint.getImageFilter() ? '!' : '=');
}
}  // END FIDDLE
