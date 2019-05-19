#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=100b4cbd5dd7406804e40035833a433c
REG_FIDDLE(Region_contains_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath xPath;
    paint.getTextPath("X", 1, 20, 110, &xPath);
    SkRegion xRegion;
    SkIRect drawBounds = {0, 0, 128, 128};
    xRegion.setPath(xPath, SkRegion(drawBounds));
    xRegion.op(drawBounds, SkRegion::kReverseDifference_Op);
    canvas->drawRegion(xRegion, paint);
    SkIRect test = SkIRect::MakeXYWH(frame* 128, 64, 5, 5);
    if (xRegion.contains(test)) {
        paint.setColor(SK_ColorYELLOW);
        canvas->drawRect(SkRect::Make(test), paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
