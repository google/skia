#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=63e32dec4b2d8440b427f368bf8313a4
REG_FIDDLE(Path_rMoveTo, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.addRect({20, 20, 80, 80}, SkPath::kCW_Direction, 2);
    path.rMoveTo(25, 2);
    SkVector arrow[] = {{0, -4}, {-20, 0}, {0, -3}, {-5, 5}, {5, 5}, {0, -3}, {20, 0}};
    for (unsigned i = 0; i < SK_ARRAY_COUNT(arrow); ++i) {
        path.rLineTo(arrow[i].fX, arrow[i].fY);
    }
    SkPaint paint;
    canvas->drawPath(path, paint);
    SkPoint lastPt;
    path.getLastPt(&lastPt);
    canvas->drawString("start", lastPt.fX, lastPt.fY, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
