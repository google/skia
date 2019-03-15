#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2890ad644f980637837e6fcb386fb462
REG_FIDDLE(Paint_021, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char testStr[] = "xxxx xxxx";
    for (auto linearText : { false, true } ) {
        paint.setLinearText(linearText);
        paint.setTextSize(24);
        canvas->drawString(paint.isLinearText() ? "linear" : "hinted", 128, 30, paint);
        for (SkScalar textSize = 8; textSize < 30; textSize *= 1.22f) {
            paint.setTextSize(textSize);
            canvas->translate(0, textSize);
            canvas->drawString(testStr, 10, 0, paint);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
