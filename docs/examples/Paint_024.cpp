#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a77bbc1a4e3be9a8ab0f842f877c5ee4
REG_FIDDLE(Paint_setSubpixelText, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setSubpixelText(true);
    paint2.setFlags(paint2.getFlags() | SkPaint::kSubpixelText_Flag);
    SkDebugf("paint1 %c= paint2\n", paint1 == paint2 ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
