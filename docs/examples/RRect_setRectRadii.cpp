// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=340d6c51efaa1f7f3d0dcaf8b0e90696
REG_FIDDLE(RRect_setRectRadii, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeCap(SkPaint::kSquare_Cap);
    paint.setAntiAlias(true);
    float intervals[] = { 5, 21.75f };
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
    SkPath path;
    SkRRect rrect;
    SkVector corners[] = {{15, 17}, {17, 19}, {19, 15}, {15, 15}};
    rrect.setRectRadii({20, 20, 100, 100}, corners);
    path.addRRect(rrect, SkPath::kCW_Direction);
    canvas->drawPath(path, paint);
    path.rewind();
    path.addRRect(rrect, SkPath::kCCW_Direction, 1);
    canvas->translate(120, 0);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
