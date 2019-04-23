// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=871b0da9b4a23de11ae7a772ce14aed3
REG_FIDDLE(Canvas_drawRect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPoint rectPts[] = { {64, 48}, {192, 160} };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    SkMatrix rotator;
    rotator.setRotate(30, 128, 128);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorMAGENTA } ) {
        paint.setColor(color);
        SkRect rect;
        rect.set(rectPts[0], rectPts[1]);
        canvas->drawRect(rect, paint);
        rotator.mapPoints(rectPts, 2);
    }
}
}  // END FIDDLE
