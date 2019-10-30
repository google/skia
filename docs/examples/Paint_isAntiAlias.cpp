#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d7d5f4f7da7acd5104a652f490c6f7b8
REG_FIDDLE(Paint_isAntiAlias, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("paint.isAntiAlias() %c= !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)\n",
            paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag) ? '=' : '!');
    paint.setAntiAlias(true);
    SkDebugf("paint.isAntiAlias() %c= !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)\n",
            paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
