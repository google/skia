// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=77e4394caf9fa083c19c21c2462efe14
REG_FIDDLE(Path_isNestedFillRects, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);
    SkPath path;
    path.addRect({10, 20, 30, 40});
    paint.getFillPath(path, &path);
    SkRect rects[2];
    SkPath::Direction directions[2];
    if (path.isNestedFillRects(rects, directions)) {
        for (int i = 0; i < 2; ++i) {
            SkDebugf("%s (%g, %g, %g, %g); direction %s\n", i ? "inner" : "outer",
                     rects[i].fLeft, rects[i].fTop, rects[i].fRight, rects[i].fBottom,
                     SkPath::kCW_Direction == directions[i] ? "CW" : "CCW");
        }
    } else {
        SkDebugf("is not nested rectangles\n");
    }
}
}  // END FIDDLE
