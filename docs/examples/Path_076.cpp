// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9235f6309271d6420fa5c45dc28664c5
REG_FIDDLE(Path_076, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    SkPath path;
    const SkPoint points[] = {{20, 20}, {70, 20}, {40, 90}};
    path.addPoly(points, SK_ARRAY_COUNT(points), false);
    for (int loop = 0; loop < 2; ++loop) {
        for (auto style : {SkPaint::kStroke_Style, SkPaint::kFill_Style,
                SkPaint::kStrokeAndFill_Style} ) {
            paint.setStyle(style);
            canvas->drawPath(path, paint);
            canvas->translate(85, 0);
        }
        path.close();
        canvas->translate(-255, 128);
    }
}
}  // END FIDDLE
