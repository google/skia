// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3b1aebacc21c1836a52876b9b0b3905e
REG_FIDDLE(Paint_057, 462, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(10, 50);
    path.quadTo(35, 110, 60, 210);
    path.quadTo(105, 110, 130, 10);
    SkPaint paint;  // set to default kMiter_Join
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    canvas->drawPath(path, paint);
    canvas->translate(150, 0);
    paint.setStrokeJoin(SkPaint::kBevel_Join);
    canvas->drawPath(path, paint);
    canvas->translate(150, 0);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
