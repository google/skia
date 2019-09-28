// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=917c44b504d3f9308571fd3835d90a0d
REG_FIDDLE(Paint_setStrokeCap_b, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    SkPath path;
    path.moveTo(30, 20);
    path.lineTo(40, 40);
    path.conicTo(70, 20, 100, 20, .707f);
    for (SkPaint::Join j : { SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join } ) {
        paint.setStrokeJoin(j);
        canvas->drawPath(path, paint);
        canvas->translate(0, 70);
    }
}
}  // END FIDDLE
