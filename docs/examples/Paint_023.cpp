#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=abe9afc0932e2199324ae6cbb396e67c
REG_FIDDLE(Paint_023, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("paint.isSubpixelText() %c= !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)\n",
        paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag) ? '=' : '!');
    paint.setSubpixelText(true);
    SkDebugf("paint.isSubpixelText() %c= !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)\n",
        paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
