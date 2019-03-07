#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=46de22da2f3e08a8d7f064634fc1c7b5
REG_FIDDLE(Region_039, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath xPath, testPath;
    paint.getTextPath("X", 1, 20, 110, &xPath);
    paint.getTextPath("`", 1, frame * 150 - 40, 150, &testPath);
    SkRegion xRegion, testRegion;
    SkIRect drawBounds = {0, 0, 128, 128};
    xRegion.setPath(xPath, SkRegion(drawBounds));
    testRegion.setPath(testPath, SkRegion(drawBounds));
    xRegion.op(drawBounds, SkRegion::kReverseDifference_Op);
    canvas->drawRegion(xRegion, paint);
    if (xRegion.contains(testRegion)) {
        paint.setColor(SK_ColorYELLOW);
        canvas->drawRegion(testRegion, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
