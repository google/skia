// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=69cdea599dcaaec35efcb24403f4287b
REG_FIDDLE(Matrix_setRectToRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const SkRect srcs[] = { {0, 0, 0, 0}, {1, 2, 3, 4} };
    const SkRect dsts[] = { {0, 0, 0, 0}, {5, 6, 8, 9} };
    for (auto src : srcs) {
        for (auto dst : dsts) {
             SkMatrix matrix;
             matrix.setAll(-1, -1, -1, -1, -1, -1, -1, -1, -1);
             bool success = matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
             SkDebugf("src: %g, %g, %g, %g  dst: %g, %g, %g, %g  success: %s\n",
                      src.fLeft, src.fTop, src.fRight, src.fBottom,
                      dst.fLeft, dst.fTop, dst.fRight, dst.fBottom, success ? "true" : "false");
             matrix.dump();
        }
    }
}
}  // END FIDDLE
