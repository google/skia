#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=68e1fd95dd2fd06a333899d2bd2396b9
REG_FIDDLE(Paint_025, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("paint.isLCDRenderText() %c= !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)\n",
        paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag) ? '=' : '!');
    paint.setLCDRenderText(true);
    SkDebugf("paint.isLCDRenderText() %c= !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)\n",
        paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
