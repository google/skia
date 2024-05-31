// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_getStyle, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("SkPaint::kFill_Style %c= paint.getStyle()\n",
            SkPaint::kFill_Style == paint.getStyle() ? '=' : '!');
}
}  // END FIDDLE
