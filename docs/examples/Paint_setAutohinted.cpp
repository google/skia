#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4e185306d7de9390fe8445eed0139309
REG_FIDDLE(Paint_setAutohinted, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char testStr[] = "xxxx xxxx";
        for (auto forceAutoHinting : { false, true} ) {
        paint.setAutohinted(forceAutoHinting);
        paint.setTextSize(24);
        canvas->drawString(paint.isAutohinted() ? "auto-hinted" : "default", 108, 30, paint);
        for (SkScalar textSize = 8; textSize < 30; textSize *= 1.22f) {
            paint.setTextSize(textSize);
            canvas->translate(0, textSize);
            canvas->drawString(testStr, 10, 0, paint);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
