#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f4ce93f6c5e7335436a985377fd980c0
REG_FIDDLE(Paint_isDither, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("paint.isDither() %c= !!(paint.getFlags() & SkPaint::kDither_Flag)\n",
            paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag) ? '=' : '!');
    paint.setDither(true);
    SkDebugf("paint.isDither() %c= !!(paint.getFlags() & SkPaint::kDither_Flag)\n",
            paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
