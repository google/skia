#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5acc77eba0cb4d00bbf3a8f4db0c0aee
REG_FIDDLE(Arc, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkRect oval = {8, 8, 56, 56};
    SkPaint ovalPaint;
    ovalPaint.setAntiAlias(true);
    SkPaint textPaint(ovalPaint);
    ovalPaint.setStyle(SkPaint::kStroke_Style);
    SkPaint arcPaint(ovalPaint);
    arcPaint.setStrokeWidth(5);
    arcPaint.setColor(SK_ColorBLUE);
    canvas->translate(-64, 0);
    for (char arcStyle = '1'; arcStyle <= '6'; ++arcStyle) {
        '4' == arcStyle ? canvas->translate(-96, 55) : canvas->translate(64, 0);
        canvas->drawText(&arcStyle, 1, 30, 36, textPaint);
        canvas->drawOval(oval, ovalPaint);
        SkPath path;
        path.moveTo({56, 32});
        switch (arcStyle) {
            case '1':
                path.arcTo(oval, 0, 90, false);
                break;
            case '2':
                canvas->drawArc(oval, 0, 90, false, arcPaint);
                continue;
            case '3':
                path.addArc(oval, 0, 90);
                break;
            case '4':
                path.arcTo({56, 56}, {32, 56}, 24);
                break;
            case '5':
                path.arcTo({24, 24}, 0, SkPath::kSmall_ArcSize, SkPath::kCW_Direction, {32, 56});
                break;
            case '6':
                path.conicTo({56, 56}, {32, 56}, SK_ScalarRoot2Over2);
                break;
         }
         canvas->drawPath(path, arcPaint);
     }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
