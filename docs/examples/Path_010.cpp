// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=71fc6c069c377d808799f2453edabaf5
REG_FIDDLE(Path_010, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
   SkPath path;
   path.addRect({10, 10, 30, 30}, SkPath::kCW_Direction);
   path.addRect({20, 20, 40, 40}, SkPath::kCW_Direction);
   path.addRect({10, 60, 30, 80}, SkPath::kCW_Direction);
   path.addRect({20, 70, 40, 90}, SkPath::kCCW_Direction);
   SkPaint strokePaint;
   strokePaint.setStyle(SkPaint::kStroke_Style);
   SkRect clipRect = {0, 0, 51, 100};
   canvas->drawPath(path, strokePaint);
   SkPaint fillPaint;
   for (auto fillType : { SkPath::kWinding_FillType, SkPath::kEvenOdd_FillType,
                      SkPath::kInverseWinding_FillType, SkPath::kInverseEvenOdd_FillType } ) {
        canvas->translate(51, 0);
        canvas->save();
        canvas->clipRect(clipRect);
        path.setFillType(fillType);
        canvas->drawPath(path, fillPaint);
        canvas->restore();
    }
}
}  // END FIDDLE
