// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e6d8ca0cc17e0b475bd54dd995825468
REG_FIDDLE(Paint_getFillPath_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    SkPath strokePath;
    strokePath.moveTo(20, 20);
    strokePath.lineTo(100, 100);
    canvas->drawPath(strokePath, paint);
    SkPath fillPath;
    paint.getFillPath(strokePath, &fillPath);
    paint.setStrokeWidth(2);
    canvas->translate(40, 0);
    canvas->drawPath(fillPath, paint);
}
}  // END FIDDLE
