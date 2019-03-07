// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a993831c40f3e134f809134e3b74e4a6
REG_FIDDLE(Paint_082, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 10));
    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);
}
}  // END FIDDLE
