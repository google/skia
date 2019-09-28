// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=99aa73f64df8bbf06e656cd891a81b9e
REG_FIDDLE(Paint_getStrokeWidth, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("0 %c= paint.getStrokeWidth()\n", 0 == paint.getStrokeWidth() ? '=' : '!');
}
}  // END FIDDLE
