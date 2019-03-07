// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c653d9017983d2a047b1fee6a481d82b
REG_FIDDLE(Rect_012, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fTop: %g unsorted.y(): %g\n", unsorted.fTop, unsorted.y());
    SkRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fTop: %g sorted.y(): %g\n", sorted.fTop, sorted.y());
}
}  // END FIDDLE
