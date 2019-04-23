#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=424741e26e1b174e43087d67422ce14f
REG_FIDDLE(Paint_getFontSpacing, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    for (SkScalar textSize : { 12, 18, 24, 32 } ) {
        paint.setTextSize(textSize);
        SkDebugf("textSize: %g fontSpacing: %g\n", textSize, paint.getFontSpacing());
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
