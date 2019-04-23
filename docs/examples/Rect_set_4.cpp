// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ee72450381f768f3869153cdbeccdc3e
REG_FIDDLE(Rect_set_4, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint point1 = {SK_ScalarNaN, 8};
    SkPoint point2 = {3, 4};
    SkRect rect;
    rect.set(point1, point2);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    rect.set(point2, point1);
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
