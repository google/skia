// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=23c77a35ac54a439a2989f840aa5cb99
REG_FIDDLE(Rect_x, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 5, 10, 25 };
    SkDebugf("unsorted.fLeft: %g unsorted.x(): %g\n", unsorted.fLeft, unsorted.x());
    SkRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fLeft: %g sorted.x(): %g\n", sorted.fLeft, sorted.x());
}
}  // END FIDDLE
