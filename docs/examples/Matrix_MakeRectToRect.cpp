// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a1d6a6721b39350f81021f71a1b93208
REG_FIDDLE(Matrix_MakeRectToRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const SkRect srcs[] = { {0, 0, 0, 0}, {1, 2, 3, 4} };
    const SkRect dsts[] = { {0, 0, 0, 0}, {5, 6, 8, 9} };
    for (auto src : srcs) {
        for (auto dst : dsts) {
             SkMatrix matrix = SkMatrix::MakeRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
             SkDebugf("src: %g, %g, %g, %g  dst: %g, %g, %g, %g\n",
                      src.fLeft, src.fTop, src.fRight, src.fBottom,
                      dst.fLeft, dst.fTop, dst.fRight, dst.fBottom);
             matrix.dump();
        }
    }
}
}  // END FIDDLE
