// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8cf5684b187d60f09e11c4a48993ea39
REG_FIDDLE(Path_Effect_Methods, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(16);
    SkScalar intervals[] = {30, 10};
    paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 1));
    canvas->drawRoundRect({20, 20, 120, 120}, 20, 20, paint);
}
}  // END FIDDLE
