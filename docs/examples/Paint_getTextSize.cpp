#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=983e2a71ba72d4ba8c945420040b8f1c
REG_FIDDLE(Paint_getTextSize, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("12 %c= default text size\n", 12 == paint.getTextSize() ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
