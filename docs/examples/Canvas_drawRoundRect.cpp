// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=199fe818c09026c114e165bff166a39f
REG_FIDDLE(Canvas_drawRoundRect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkVector radii[] = { {0, 20}, {10, 10}, {10, 20}, {10, 40} };
    SkPaint paint;
    paint.setStrokeWidth(15);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setAntiAlias(true);
    for (auto style : { SkPaint::kStroke_Style, SkPaint::kFill_Style  } ) {
        paint.setStyle(style );
        for (size_t i = 0; i < SK_ARRAY_COUNT(radii); ++i) {
           canvas->drawRoundRect({10, 10, 60, 40}, radii[i].fX, radii[i].fY, paint);
           canvas->translate(0, 60);
        }
        canvas->translate(80, -240);
    }
}
}  // END FIDDLE
