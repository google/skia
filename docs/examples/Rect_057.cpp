// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a476548d0001296afd8e58c1eba1b70b
REG_FIDDLE(Rect_057, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 100, 15, 0};
    SkRect sorted = rect.makeSorted();
    SkRect toJoin = { 50, 60, 55, 65 };
    rect.joinPossiblyEmptyRect(toJoin);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    sorted.joinPossiblyEmptyRect(toJoin);
    SkDebugf("sorted: %g, %g, %g, %g\n", sorted.fLeft, sorted.fTop, sorted.fRight, sorted.fBottom);
}
}  // END FIDDLE
