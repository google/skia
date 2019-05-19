#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=06084f609184470135a9cd9ebc5af149
REG_FIDDLE(Paint_measureText, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(50);
    const char str[] = "ay^jZ";
    const int count = sizeof(str) - 1;
    canvas->drawText(str, count, 25, 50, paint);
    SkRect bounds;
    paint.measureText(str, count, &bounds);
    canvas->translate(25, 50);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(bounds, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
