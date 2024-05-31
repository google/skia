// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_clipPath_3, 256, 212, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    SkPoint poly[] = {{20, 20}, { 80, 20}, { 80,  80}, {40,  80},
                      {40, 40}, {100, 40}, {100, 100}, {20, 100}};
    path.addPoly(poly, std::size(poly), true);
    path.setFillType(SkPathFillType::kWinding);
    canvas->save();
    canvas->clipPath(path, SkClipOp::kIntersect);
    canvas->drawCircle(50, 50, 45, paint);
    canvas->restore();
    canvas->translate(100, 100);
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->clipPath(path, SkClipOp::kIntersect);
    canvas->drawCircle(50, 50, 45, paint);
}
}  // END FIDDLE
