// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3cfc24b011aef1ca8ccb57c05711620c
REG_FIDDLE(Rect_014, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fTop: %g unsorted.top(): %g\n", unsorted.fTop, unsorted.top());
    SkRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fTop: %g sorted.top(): %g\n", sorted.fTop, sorted.top());
}
}  // END FIDDLE
