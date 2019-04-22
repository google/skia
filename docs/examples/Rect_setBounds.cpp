// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=cf0da15f48aa54fd1889e7f913601710
REG_FIDDLE(Rect_setBounds, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint points[] = {{3, 4}, {1, 2}, {5, 6}, {SK_ScalarNaN, 8}};
    for (int count = 0; count <= (int) SK_ARRAY_COUNT(points); ++count) {
        SkRect rect;
        rect.setBounds(points, count);
        if (count > 0) {
            SkDebugf("added: %3g, %g ", points[count - 1].fX,  points[count - 1].fY);
        } else {
            SkDebugf("%14s", " ");
        }
        SkDebugf("count: %d rect: %g, %g, %g, %g\n", count,
                rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    }
}
}  // END FIDDLE
