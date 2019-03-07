// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2bffb6384cc20077e632e7d01da045ca
REG_FIDDLE(Paint_053, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    SkPath path;
    path.moveTo(30, 30);
    path.lineTo(30, 30);
    path.moveTo(70, 30);
    path.lineTo(90, 40);
    for (SkPaint::Cap c : { SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap } ) {
        paint.setStrokeCap(c);
        canvas->drawPath(path, paint);
        canvas->translate(0, 70);
    }
}
}  // END FIDDLE
