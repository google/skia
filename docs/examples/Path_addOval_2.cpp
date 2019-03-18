#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f1122d6fffddac0167e96fab4b9a862f
REG_FIDDLE(Path_addOval_2, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    const SkPoint arrow[] = { {0, -5}, {10, 0}, {0, 5} };
    const SkRect rect = {10, 10, 54, 54};
    SkPaint ovalPaint;
    ovalPaint.setAntiAlias(true);
    SkPaint textPaint(ovalPaint);
    ovalPaint.setStyle(SkPaint::kStroke_Style);
    SkPaint arrowPaint(ovalPaint);
    SkPath arrowPath;
    arrowPath.addPoly(arrow, SK_ARRAY_COUNT(arrow), true);
    arrowPaint.setPathEffect(SkPath1DPathEffect::Make(arrowPath, 176, 0,
                             SkPath1DPathEffect::kRotate_Style));
    for (auto direction : { SkPath::kCW_Direction, SkPath::kCCW_Direction } ) {
        for (unsigned start : { 0, 1, 2, 3 } ) {
           SkPath path;
           path.addOval(rect, direction, start);
           canvas->drawPath(path, ovalPaint);
           canvas->drawPath(path, arrowPaint);
           canvas->drawText(&"0123"[start], 1, rect.centerX(), rect.centerY() + 5, textPaint);
           canvas->translate(64, 0);
       }
       canvas->translate(-256, 72);
       canvas->drawString(SkPath::kCW_Direction == direction ? "clockwise" : "counterclockwise",
                          128, 0, textPaint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
