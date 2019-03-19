// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cbec1ae6530e95943775450b1d11f19e
REG_FIDDLE(IRect_top, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fTop: %d unsorted.top(): %d\n", unsorted.fTop, unsorted.top());
    SkIRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fTop: %d sorted.top(): %d\n", sorted.fTop, sorted.top());
}
}  // END FIDDLE
