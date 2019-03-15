#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d2c33dc791cd165dcc2423226ba5b095
REG_FIDDLE(Path_011, 256, 230, false, 0) {
void draw(SkCanvas* canvas) {
   SkPath path;
   path.addRect({20, 10, 80, 70}, SkPath::kCW_Direction);
   path.addRect({40, 30, 100, 90}, SkPath::kCW_Direction);
   SkPaint strokePaint;
   strokePaint.setStyle(SkPaint::kStroke_Style);
   SkRect clipRect = {0, 0, 128, 128};
   canvas->drawPath(path, strokePaint);
   canvas->drawLine({0, 50}, {120, 50}, strokePaint);
   SkPaint textPaint;
   textPaint.setAntiAlias(true);
   SkScalar textHPos[] = { 10, 30, 60, 90, 110 };
   canvas->drawPosTextH("01210", 5, textHPos, 48, textPaint);
   textPaint.setTextSize(18);
   canvas->translate(0, 128);
   canvas->scale(.5f, .5f);
   canvas->drawString("inverse", 384, 150, textPaint);
   SkPaint fillPaint;
   for (auto fillType : { SkPath::kWinding_FillType, SkPath::kEvenOdd_FillType,
                      SkPath::kInverseWinding_FillType, SkPath::kInverseEvenOdd_FillType } ) {
        canvas->save();
        canvas->clipRect(clipRect);
        path.setFillType(fillType);
        canvas->drawPath(path, fillPaint);
        canvas->restore();
        canvas->drawString(fillType & 1 ? "even-odd" : "winding", 64, 170, textPaint);
        canvas->translate(128, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
