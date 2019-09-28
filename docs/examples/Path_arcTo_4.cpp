// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6b6ea44f659b27918f3a6fa621bf6173
REG_FIDDLE(Path_arcTo_4, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    for (auto sweep: { SkPath::kCW_Direction, SkPath::kCCW_Direction } ) {
        for (auto arcSize : { SkPath::kSmall_ArcSize, SkPath::kLarge_ArcSize } ) {
            SkPath path;
            path.moveTo({120, 50});
            path.arcTo(70, 40, 30, arcSize, sweep, 120.1f, 50);
            if (SkPath::kCCW_Direction == sweep && SkPath::kLarge_ArcSize == arcSize) {
                paint.setColor(SK_ColorBLUE);
                paint.setStrokeWidth(3);
            }
            canvas->drawPath(path, paint);
         }
    }
}
}  // END FIDDLE
