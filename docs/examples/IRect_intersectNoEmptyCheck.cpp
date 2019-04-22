// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d35fbc9fdea71df8b8a12fd3da50d11c
REG_FIDDLE(IRect_intersectNoEmptyCheck, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect result;
    if (result.intersectNoEmptyCheck({ 10, 40, 50, 80 }, { 30, 60, 70, 90 })) {
        SkDebugf("intersection: %d, %d, %d, %d\n",
                 result.left(), result.top(), result.right(), result.bottom());
    }
}
}  // END FIDDLE
