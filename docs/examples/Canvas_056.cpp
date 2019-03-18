// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7856755c1bf8431c286c734b353345ad
REG_FIDDLE(Canvas_clipPath_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.addRect({20, 15, 100, 95});
    path.addRect({50, 65, 130, 135});
    path.setFillType(SkPath::kWinding_FillType);
    canvas->save();
    canvas->clipPath(path, SkClipOp::kIntersect);
    canvas->drawCircle(70, 85, 60, paint);
    canvas->restore();
    canvas->translate(100, 100);
    path.setFillType(SkPath::kEvenOdd_FillType);
    canvas->clipPath(path, SkClipOp::kIntersect);
    canvas->drawCircle(70, 85, 60, paint);
}
}  // END FIDDLE
