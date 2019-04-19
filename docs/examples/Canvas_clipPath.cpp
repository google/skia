// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ee47ae6b813bfaa55e1a7b7c053ed60d
REG_FIDDLE(Canvas_clipPath, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.addRect({20, 30, 100, 110});
    path.setFillType(SkPath::kInverseWinding_FillType);
    canvas->save();
    canvas->clipPath(path, SkClipOp::kDifference, false);
    canvas->drawCircle(70, 100, 60, paint);
    canvas->restore();
    canvas->translate(100, 100);
    path.setFillType(SkPath::kWinding_FillType);
    canvas->clipPath(path, SkClipOp::kIntersect, false);
    canvas->drawCircle(70, 100, 60, paint);
}
}  // END FIDDLE
