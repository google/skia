#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=2e3497890d523235f96680716c321098
REG_FIDDLE(Region_op_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath xPath;
    paint.getTextPath("X", 1, 20, 110, &xPath);
    SkRegion xRegion;
    SkIRect drawBounds = {0, 0, 128, 128};
    xRegion.setPath(xPath, SkRegion(drawBounds));
    xRegion.op(drawBounds.fLeft + frame * drawBounds.width(), drawBounds.fTop,
               drawBounds.fRight, drawBounds.fBottom, SkRegion::kReverseDifference_Op);
    canvas->drawRegion(xRegion, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
