// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=11f0fbe7b30d776913c2e7c92c02ff57
REG_FIDDLE(Canvas_drawArc_a, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect oval = { 4, 4, 60, 60};
    for (auto useCenter : { false, true } ) {
        for (auto style : { SkPaint::kFill_Style, SkPaint::kStroke_Style } ) {
            paint.setStyle(style);
            for (auto degrees : { 45, 90, 180, 360} ) {
                canvas->drawArc(oval, 0, degrees , useCenter, paint);
                canvas->translate(64, 0);
            }
            canvas->translate(-256, 64);
        }
    }
}
}  // END FIDDLE
