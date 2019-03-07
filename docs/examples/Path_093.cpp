// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=182b3999772f330f3b0b891b492634ae
REG_FIDDLE(Path_093, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    const SkPoint points[] = {{20, 20}, {70, 20}, {40, 90}};
    for (bool close : { false, true } ) {
        SkPath path;
        path.addPoly(points, SK_ARRAY_COUNT(points), close);
        for (auto style : {SkPaint::kStroke_Style, SkPaint::kFill_Style,
                SkPaint::kStrokeAndFill_Style} ) {
            paint.setStyle(style);
            canvas->drawPath(path, paint);
            canvas->translate(85, 0);
        }
        canvas->translate(-255, 128);
    }
}
}  // END FIDDLE
