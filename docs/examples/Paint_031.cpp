#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e811f4829a2daaaeaad3795504a7e02a
REG_FIDDLE(Fake_Bold, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(40);
    canvas->drawString("OjYy_-", 10, 35, paint);
    paint.setFakeBoldText(true);
    canvas->drawString("OjYy_-", 10, 75, paint);
    // create a custom fake bold by varying the stroke width
    paint.setFakeBoldText(false);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(40.f / 48);
    canvas->drawString("OjYy_-", 10, 115, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
