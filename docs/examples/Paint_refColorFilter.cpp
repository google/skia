// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_refColorFilter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setColorFilter(SkColorFilters::Blend(0xFFFF0000, SkBlendMode::kSrcATop));
    SkDebugf("color filter unique: %s\n", paint1.getColorFilter()->unique() ? "true" : "false");
    paint2.setColorFilter(paint1.refColorFilter());
    SkDebugf("color filter unique: %s\n", paint1.getColorFilter()->unique() ? "true" : "false");
}
}  // END FIDDLE
