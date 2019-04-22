// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ca3de7e5e292b3ad3633b1c39a31d3ab
REG_FIDDLE(Rect_right, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fRight: %g unsorted.right(): %g\n", unsorted.fRight, unsorted.right());
    SkRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fRight: %g sorted.right(): %g\n", sorted.fRight, sorted.right());
}
}  // END FIDDLE
