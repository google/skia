// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6ea461e71f7fc80605818fbf493caa63
REG_FIDDLE(IRect_010, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted.fTop: %d unsorted.y(): %d\n", unsorted.fTop, unsorted.y());
    SkIRect sorted = unsorted.makeSorted();
    SkDebugf("sorted.fTop: %d sorted.y(): %d\n", sorted.fTop, sorted.y());
}
}  // END FIDDLE
