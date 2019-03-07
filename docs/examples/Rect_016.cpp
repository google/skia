// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a98993a66616ae406d8bdc54adfb1411
REG_FIDDLE(Rect_016, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fBottom: %g unsorted.bottom(): %g\n", unsorted.fBottom, unsorted.bottom());
    SkRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fBottom: %g sorted.bottom(): %g\n", sorted.fBottom, sorted.bottom());
}
}  // END FIDDLE
