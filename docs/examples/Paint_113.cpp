#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f1139a5ddd17fd47c2f45f6e642cac76
REG_FIDDLE(Paint_113, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("default width = %g\n", paint.measureText("!", 1));
    paint.setTextSize(paint.getTextSize() * 2);
    SkDebugf("double width = %g\n", paint.measureText("!", 1));
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
