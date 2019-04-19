// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fa12547fcfd4c1aef3db1a1f6aae0fe4
REG_FIDDLE(IRect_sort, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 30, 50, 20, 10 };
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    rect.sort();
    SkDebugf("sorted: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
