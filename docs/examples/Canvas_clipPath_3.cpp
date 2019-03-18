// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=187a7ae77a8176e417181411988534b6
REG_FIDDLE(Canvas_clipPath_3, 256, 212, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    SkPoint poly[] = {{20, 20}, { 80, 20}, { 80,  80}, {40,  80},
                      {40, 40}, {100, 40}, {100, 100}, {20, 100}};
    path.addPoly(poly, SK_ARRAY_COUNT(poly), true);
    path.setFillType(SkPath::kWinding_FillType);
    canvas->save();
    canvas->clipPath(path, SkClipOp::kIntersect);
    canvas->drawCircle(50, 50, 45, paint);
    canvas->restore();
    canvas->translate(100, 100);
    path.setFillType(SkPath::kEvenOdd_FillType);
    canvas->clipPath(path, SkClipOp::kIntersect);
    canvas->drawCircle(50, 50, 45, paint);
}
}  // END FIDDLE
