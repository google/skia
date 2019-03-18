// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9202430b3f4f5275af8eec5cc9d7baa8
REG_FIDDLE(Path_addRect_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    const SkPoint arrow[] = { {5, -5}, {15, -5}, {20, 0}, {15, 5}, {5, 5}, {10, 0} };
    const SkRect rect = {10, 10, 54, 54};
    SkPaint rectPaint;
    rectPaint.setAntiAlias(true);
    rectPaint.setStyle(SkPaint::kStroke_Style);
    SkPaint arrowPaint(rectPaint);
    SkPath arrowPath;
    arrowPath.addPoly(arrow, SK_ARRAY_COUNT(arrow), true);
    arrowPaint.setPathEffect(SkPath1DPathEffect::Make(arrowPath, 176, 0,
                             SkPath1DPathEffect::kRotate_Style));
    for (auto direction : { SkPath::kCW_Direction, SkPath::kCCW_Direction } ) {
        for (unsigned start : { 0, 1, 2, 3 } ) {
           SkPath path;
           path.addRect(rect, direction, start);
           canvas->drawPath(path, rectPaint);
           canvas->drawPath(path, arrowPaint);
           canvas->translate(64, 0);
       }
       canvas->translate(-256, 64);
    }
}
}  // END FIDDLE
