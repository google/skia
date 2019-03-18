// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e624fe398e3d770b573c09fc74c0c400
REG_FIDDLE(Rect_sort, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 20.5f, 10.5f };
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    rect.sort();
    SkDebugf("sorted: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
