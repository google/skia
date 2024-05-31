// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_FillType_a, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
   SkPath path;
   path.addRect({10, 10, 30, 30}, SkPathDirection::kCW);
   path.addRect({20, 20, 40, 40}, SkPathDirection::kCW);
   path.addRect({10, 60, 30, 80}, SkPathDirection::kCW);
   path.addRect({20, 70, 40, 90}, SkPathDirection::kCCW);
   SkPaint strokePaint;
   strokePaint.setStyle(SkPaint::kStroke_Style);
   SkRect clipRect = {0, 0, 51, 100};
   canvas->drawPath(path, strokePaint);
   SkPaint fillPaint;
   for (auto fillType : { SkPathFillType::kWinding, SkPathFillType::kEvenOdd,
                      SkPathFillType::kInverseWinding, SkPathFillType::kInverseEvenOdd } ) {
        canvas->translate(51, 0);
        canvas->save();
        canvas->clipRect(clipRect);
        path.setFillType(fillType);
        canvas->drawPath(path, fillPaint);
        canvas->restore();
    }
}
}  // END FIDDLE
