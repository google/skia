#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c93bb912f3bddfb4d96d3ad70ada552b
REG_FIDDLE(Paint_022, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char testStr[] = "abcd efgh";
    for (int textSize : { 12, 24 } ) {
        paint.setTextSize(textSize);
        for (auto linearText : { false, true } ) {
            paint.setLinearText(linearText);
            SkString width;
            width.appendScalar(paint.measureText(testStr, SK_ARRAY_COUNT(testStr), nullptr));
            canvas->translate(0, textSize + 4);
            canvas->drawString(testStr, 10, 0, paint);
            canvas->drawString(width, 128, 0, paint);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
