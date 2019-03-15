#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=11c10f466dae0d1639dbb9f6a0040218
REG_FIDDLE(Paint_102, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("0 %c= default text skew x\n", 0 == paint.getTextSkewX() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
