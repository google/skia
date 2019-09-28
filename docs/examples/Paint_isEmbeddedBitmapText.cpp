#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=eba10b27b790e87183ae451b3fc5c4b1
REG_FIDDLE(Paint_isEmbeddedBitmapText, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("paint.isEmbeddedBitmapText() %c="
            " !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)\n",
            paint.isEmbeddedBitmapText() ==
            !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag) ? '=' : '!');
    paint.setEmbeddedBitmapText(true);
    SkDebugf("paint.isEmbeddedBitmapText() %c="
            " !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)\n",
            paint.isEmbeddedBitmapText() ==
            !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
