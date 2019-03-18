// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2a3782c33f04ed17a725d0e449c6f7c3
REG_FIDDLE(Paint_refDrawLooper, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    SkLayerDrawLooper::Builder looperBuilder;
    paint1.setDrawLooper(looperBuilder.detach());
    SkDebugf("draw looper unique: %s\n", paint1.getDrawLooper()->unique() ? "true" : "false");
    paint2.setDrawLooper(paint1.refDrawLooper());
    SkDebugf("draw looper unique: %s\n", paint1.getDrawLooper()->unique() ? "true" : "false");
}
}  // END FIDDLE
