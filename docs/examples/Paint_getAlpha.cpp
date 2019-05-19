// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9a85bb62fe3d877b18fb7f952c4fa7f7
REG_FIDDLE(Paint_getAlpha, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("255 %c= paint.getAlpha()\n", 255 == paint.getAlpha() ? '=' : '!');
}
}  // END FIDDLE
