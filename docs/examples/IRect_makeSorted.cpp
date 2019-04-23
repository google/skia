// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=de89926c374aa16427916900b89a3441
REG_FIDDLE(IRect_makeSorted, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 30, 50, 20, 10 };
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    SkIRect sort = rect.makeSorted();
    SkDebugf("sorted: %d, %d, %d, %d\n", sort.fLeft, sort.fTop, sort.fRight, sort.fBottom);
}
}  // END FIDDLE
