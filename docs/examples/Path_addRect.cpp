// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_addRect, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeCap(SkPaint::kSquare_Cap);
    float intervals[] = { 5, 21.75f };
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(SkDashPathEffect::Make(intervals, std::size(intervals), 0));
    SkPath path;
    path.addRect({20, 20, 100, 100}, SkPathDirection::kCW);
    canvas->drawPath(path, paint);
    path.rewind();
    path.addRect({140, 20, 220, 100}, SkPathDirection::kCCW);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
