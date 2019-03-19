// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1a6b69acad5ceafede3c5984ec6634cb
REG_FIDDLE(Path_addPoly_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    for (bool close : { false, true } ) {
        SkPath path;
        path.addPoly({{20, 20}, {70, 20}, {40, 90}}, close);
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
