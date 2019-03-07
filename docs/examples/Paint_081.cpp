// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=084b0dc3cebd78718c651d58f257f799
REG_FIDDLE(Paint_081, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1));
    SkDebugf("mask filter unique: %s\n", paint1.getMaskFilter()->unique() ? "true" : "false");
    paint2.setMaskFilter(paint1.refMaskFilter());
    SkDebugf("mask filter unique: %s\n", paint1.getMaskFilter()->unique() ? "true" : "false");
}
}  // END FIDDLE
