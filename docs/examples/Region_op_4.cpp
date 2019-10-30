#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3f964be1e1fd2fbb977b655d3a928f0a
REG_FIDDLE(Region_op_4, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath xPath, opPath;
    paint.getTextPath("X", 1, 20, 110, &xPath);
    opPath.addCircle(64, 64, frame * 64);
    SkRegion xRegion, opRegion, rectRegion;
    SkIRect drawBounds = {0, 0, 128, 128};
    opRegion.setPath(opPath, SkRegion(drawBounds));
    xRegion.setPath(xPath, SkRegion(drawBounds));
    drawBounds.inset(frame * drawBounds.width() / 2, 0);
    rectRegion.op(drawBounds, opRegion, SkRegion::kIntersect_Op);
    xRegion.op(rectRegion, SkRegion::kReverseDifference_Op);
    canvas->drawRegion(xRegion, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
