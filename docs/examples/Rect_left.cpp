// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=900dc96c3549795a87036d6458c4fde6
REG_FIDDLE(Rect_left, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 5, 10, 25 };
    SkDebugf("unsorted.fLeft: %g unsorted.left(): %g\n", unsorted.fLeft, unsorted.left());
    SkRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fLeft: %g sorted.left(): %g\n", sorted.fLeft, sorted.left());
}
}  // END FIDDLE
