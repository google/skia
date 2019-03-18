// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=97e210976f1ee0387b30c70635cf114f
REG_FIDDLE(IRect_right, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fRight: %d unsorted.right(): %d\n", unsorted.fRight, unsorted.right());
    SkIRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fRight: %d sorted.right(): %d\n", sorted.fRight, sorted.right());
}
}  // END FIDDLE
