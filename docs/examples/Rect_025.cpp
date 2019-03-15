// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=83d879b92683b15f9daaf0c9e71c5b35
REG_FIDDLE(Rect_025, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint points[] = {{3, 4}, {1, 2}, {5, 6}, {SK_ScalarNaN, 8}};
    for (int count = 0; count <= (int) SK_ARRAY_COUNT(points); ++count) {
        SkRect rect;
        bool success = rect.setBoundsCheck(points, count);
        if (count > 0) {
            SkDebugf("added: %3g, %g ", points[count - 1].fX,  points[count - 1].fY);
        } else {
            SkDebugf("%14s", " ");
        }
        SkDebugf("count: %d rect: %g, %g, %g, %g success: %s\n", count,
                rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, success ? "true" : "false");
    }
}
}  // END FIDDLE
