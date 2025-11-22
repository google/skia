// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_053, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    SkPath path = SkPathBuilder()
                  .moveTo(30, 30)
                  .lineTo(30, 30)
                  .moveTo(70, 30)
                  .lineTo(90, 40)
                  .detach();
    for (SkPaint::Cap c : { SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap } ) {
        paint.setStrokeCap(c);
        canvas->drawPath(path, paint);
        canvas->translate(0, 70);
    }
}
}  // END FIDDLE
