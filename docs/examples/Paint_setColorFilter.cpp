// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_setColorFilter, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   paint.setColorFilter(SkColorFilters::Blend(SK_ColorLTGRAY, SkBlendMode::kSrcIn));
   canvas->drawRect(SkRect::MakeWH(50, 50), paint);
   paint.setColorFilter(nullptr);
   canvas->translate(70, 0);
   canvas->drawRect(SkRect::MakeWH(50, 50), paint);
}
}  // END FIDDLE
