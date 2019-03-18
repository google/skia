#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3cad18678254526be66ef162eecd1d23
REG_FIDDLE(Font_breakText, 280, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(50);
    const char str[] = "Breakfast";
    const int count = sizeof(str) - 1;
    canvas->drawText(str, count, 25, 50, paint);
    SkScalar measuredWidth;
    SkFont font;
    font.setSize(50);
    int partialBytes = font.breakText(str, count, kUTF8_SkTextEncoding,
            100, &measuredWidth);
    canvas->drawText(str, partialBytes, 25, 100, paint);
    canvas->drawLine(25, 60, 25 + 100, 60, paint);
    canvas->drawLine(25, 110, 25 + measuredWidth, 110, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
