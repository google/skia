#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5dc8e58f6910cb8e4de9ed60f888188b
REG_FIDDLE(Paint_getTextScaleX, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("1 %c= default text scale x\n", 1 == paint.getTextScaleX() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
