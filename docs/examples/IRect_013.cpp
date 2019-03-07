// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0175bae87fafcd9433ae661574695586
REG_FIDDLE(IRect_013, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect unsorted = { 15, 25, 10, 20 };
    SkDebugf("unsorted height: %d\n", unsorted.height());
    SkIRect large = { 1, -2147483647, 2, 2147483644 };
    SkDebugf("large height: %d\n", large.height());
}
}  // END FIDDLE
