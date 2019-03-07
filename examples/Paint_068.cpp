// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Paint_068, 256, 256, true, 0);
// HASH=093bdc627d6b59002670fd290931f6c9
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= color filter\n", paint.getColorFilter() ? '!' : '=');
   paint.setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorLTGRAY, SkBlendMode::kSrcIn));
   SkDebugf("nullptr %c= color filter\n", paint.getColorFilter() ? '!' : '=');
}

}
