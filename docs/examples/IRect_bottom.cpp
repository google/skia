// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(IRect_bottom, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fBottom: %d unsorted.bottom(): %d\n", unsorted.fBottom, unsorted.bottom());
    SkIRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fBottom: %d sorted.bottom(): %d\n", sorted.fBottom, sorted.bottom());
}
}  // END FIDDLE
