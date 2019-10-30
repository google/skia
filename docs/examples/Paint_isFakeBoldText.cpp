#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f54d1f85b16073b80b9eef2e1a1d151d
REG_FIDDLE(Paint_isFakeBoldText, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("paint.isFakeBoldText() %c= !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)\n",
        paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag) ? '=' : '!');
    paint.setFakeBoldText(true);
    SkDebugf("paint.isFakeBoldText() %c= !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)\n",
        paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
