// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f59567042b87f6b26f9bfeeb04468032
REG_FIDDLE(Rect_makeSorted, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30.5f, 50.5f, 20.5f, 10.5f };
    SkDebugf("rect: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    SkRect sort = rect.makeSorted();
    SkDebugf("sorted: %g, %g, %g, %g\n", sort.fLeft, sort.fTop, sort.fRight, sort.fBottom);
}
}  // END FIDDLE
