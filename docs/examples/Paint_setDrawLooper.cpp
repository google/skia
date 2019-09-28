// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bf10f838b330f0a3a3266d42ea68a638
REG_FIDDLE(Paint_setDrawLooper, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
    SkPaint paint;
    paint.setDrawLooper(SkBlurDrawLooper::Make(0x7FFF0000, 4, -5, -10));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    paint.setAntiAlias(true);
    paint.setColor(0x7f0000ff);
    canvas->drawCircle(70, 70, 50, paint);
#endif
}
}  // END FIDDLE
