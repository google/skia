#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8a3f8c309533388b01aa66e1267f322d
REG_FIDDLE(Paint_011, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkDebugf("(SkPaint::kAntiAlias_Flag & paint.getFlags()) %c= 0\n",
        SkPaint::kAntiAlias_Flag & paint.getFlags() ? '!' : '=');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
