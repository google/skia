#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=50dedf8450159571a3edaf4f0050defe
REG_FIDDLE(Paint_setLCDRenderText, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint1.setLCDRenderText(true);
    paint2.setFlags(paint2.getFlags() | SkPaint::kLCDRenderText_Flag);
    SkDebugf("paint1 %c= paint2\n", paint1 == paint2 ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
