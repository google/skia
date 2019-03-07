// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=caf38ea4431bc246ba198f6a8c2b0f01
REG_FIDDLE(IRect_005, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 5, 10, 25 };
    SkDebugf("unsorted.fLeft: %d unsorted.left(): %d\n", unsorted.fLeft, unsorted.left());
    SkIRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fLeft: %d sorted.left(): %d\n", sorted.fLeft, sorted.left());
}
}  // END FIDDLE
