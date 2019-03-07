#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4bbae00b40ed2cfcd0007921ad693a7b
REG_FIDDLE(Path_001, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    const SkPoint arrow[] = { {40, -5}, {45, 0}, {40, 5} };
    const SkRect rect = {10, 10, 90, 90};
    SkPaint rectPaint;
    rectPaint.setAntiAlias(true);
    SkPaint textPaint(rectPaint);
    rectPaint.setStyle(SkPaint::kStroke_Style);
    SkPaint arrowPaint(rectPaint);
    SkPath arrowPath;
    arrowPath.addPoly(arrow, SK_ARRAY_COUNT(arrow), true);
    arrowPaint.setPathEffect(SkPath1DPathEffect::Make(arrowPath, 320, 0,
                             SkPath1DPathEffect::kRotate_Style));
    for (auto direction : { SkPath::kCW_Direction, SkPath::kCCW_Direction } ) {
        canvas->drawRect(rect, rectPaint);
        for (unsigned start : { 0, 1, 2, 3 } ) {
           SkPath path;
           path.addRect(rect, direction, start);
           canvas->drawPath(path, arrowPaint);
       }
       canvas->drawString(SkPath::kCW_Direction == direction ? "CW" : "CCW",  rect.centerX(),
            rect.centerY(), textPaint);
       canvas->translate(120, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
