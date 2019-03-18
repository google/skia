// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f56039b94c702c2704c8c5100e623aca
REG_FIDDLE(Paint_refPathEffect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    SkScalar intervals[] = {1, 2};
    paint1.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 10));
    SkDebugf("path effect unique: %s\n", paint1.getPathEffect()->unique() ? "true" : "false");
    paint2.setPathEffect(paint1.refPathEffect());
    SkDebugf("path effect unique: %s\n", paint1.getPathEffect()->unique() ? "true" : "false");
}
}  // END FIDDLE
