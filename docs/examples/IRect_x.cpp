// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(IRect_x, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 5, 10, 25 };
    SkDebugf("unsorted.fLeft: %d unsorted.x(): %d\n", unsorted.fLeft, unsorted.x());
    SkIRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fLeft: %d sorted.x(): %d\n", sorted.fLeft, sorted.x());
}
}  // END FIDDLE
