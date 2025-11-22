// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_ArcSize, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    for (auto sweep: { SkPathDirection::kCW, SkPathDirection::kCCW } ) {
        for (auto arcSize : { SkPathBuilder::kSmall_ArcSize, SkPathBuilder::kLarge_ArcSize } ) {
            SkPath path = SkPathBuilder()
                          .moveTo({120, 50})
                          .arcTo({70, 40}, 30, arcSize, sweep, {156, 100})
                          .detach();
            if (SkPathDirection::kCCW == sweep && SkPathBuilder::kLarge_ArcSize == arcSize) {
                paint.setColor(SK_ColorBLUE);
                paint.setStrokeWidth(3);
            }
            canvas->drawPath(path, paint);
         }
    }
}
}  // END FIDDLE
