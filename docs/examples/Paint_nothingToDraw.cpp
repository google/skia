// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2973b05bfbb6b4c29332c8ac4fcf3995
REG_FIDDLE(Paint_nothingToDraw, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPaint& p) -> void {
        SkDebugf("%s nothing to draw: %s\n", prefix,
                 p.nothingToDraw() ? "true" : "false");
    };
    SkPaint paint;
    debugster("initial", paint);
    paint.setBlendMode(SkBlendMode::kDst);
    debugster("blend dst", paint);
    paint.setBlendMode(SkBlendMode::kSrcOver);
    debugster("blend src over", paint);
    paint.setAlpha(0);
    debugster("alpha 0", paint);
}
}  // END FIDDLE
