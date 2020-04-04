// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3837827310e8b88b8c2e128ef9fbbd65
REG_FIDDLE(Path_addRect_3, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeCap(SkPaint::kSquare_Cap);
    float intervals[] = { 5, 21.75f };
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
    for (auto direction : { SkPathDirection::kCW, SkPathDirection::kCCW } ) {
        SkPath path;
        path.addRect(20, 20, 100, 100, direction);
        canvas->drawPath(path, paint);
        canvas->translate(128, 0);
    }
}
}  // END FIDDLE
