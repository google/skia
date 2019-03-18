#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6b9e101f49e9c2c28755c5bdcef64dfb
REG_FIDDLE(Paint_getTextWidths, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(50);
    const char str[] = "abc";
    const int bytes = sizeof(str) - 1;
    int count = paint.getTextWidths(str, bytes, nullptr);
    std::vector<SkScalar> widths;
    std::vector<SkRect> bounds;
    widths.resize(count);
    bounds.resize(count);
    for (int loop = 0; loop < 2; ++loop) {
        (void) paint.getTextWidths(str, count, &widths.front(), &bounds.front());
        SkPoint loc = { 25, 50 };
        canvas->drawText(str, bytes, loc.fX, loc.fY, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0);
        SkScalar advanceY = loc.fY + 10;
        for (int index = 0; index < count; ++index) {
            bounds[index].offset(loc.fX, loc.fY);
            canvas->drawRect(bounds[index], paint);
            canvas->drawLine(loc.fX, advanceY, loc.fX + widths[index], advanceY, paint);
            loc.fX += widths[index];
            advanceY += 5;
        }
        canvas->translate(0, 80);
        paint.setStrokeWidth(3);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
