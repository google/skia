// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fe2294131f422b8d6752f6a880f98ad9
REG_FIDDLE(Canvas_086, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(20, 20);
    path.quadTo(60, 20, 60, 60);
    path.close();
    path.moveTo(60, 20);
    path.quadTo(60, 60, 20, 60);
    SkPaint paint;
    paint.setStrokeWidth(10);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    for (auto join: { SkPaint::kBevel_Join, SkPaint::kRound_Join, SkPaint::kMiter_Join } ) {
        paint.setStrokeJoin(join);
        for (auto cap: { SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap  } ) {
            paint.setStrokeCap(cap);
            canvas->drawPath(path, paint);
            canvas->translate(80, 0);
        }
        canvas->translate(-240, 60);
    }
    paint.setStyle(SkPaint::kFill_Style);
    for (auto fill : { SkPath::kWinding_FillType,
                       SkPath::kEvenOdd_FillType,
                       SkPath::kInverseWinding_FillType } ) {
        path.setFillType(fill);
        canvas->save();
        canvas->clipRect({0, 10, 80, 70});
        canvas->drawPath(path, paint);
        canvas->restore();
        canvas->translate(80, 0);
    }
}
}  // END FIDDLE
