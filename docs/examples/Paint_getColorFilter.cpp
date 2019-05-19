// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=093bdc627d6b59002670fd290931f6c9
REG_FIDDLE(Paint_getColorFilter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= color filter\n", paint.getColorFilter() ? '!' : '=');
   paint.setColorFilter(SkColorFilters::Blend(SK_ColorLTGRAY, SkBlendMode::kSrcIn));
   SkDebugf("nullptr %c= color filter\n", paint.getColorFilter() ? '!' : '=');
}
}  // END FIDDLE
