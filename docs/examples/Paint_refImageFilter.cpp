// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=13f09088b569251547107d14ae989dc1
REG_FIDDLE(Paint_refImageFilter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setImageFilter(SkImageFilters::Offset(25, 25, nullptr));
    SkDebugf("image filter unique: %s\n", paint1.getImageFilter()->unique() ? "true" : "false");
    paint2.setImageFilter(paint1.refImageFilter());
    SkDebugf("image filter unique: %s\n", paint1.getImageFilter()->unique() ? "true" : "false");
}
}  // END FIDDLE
