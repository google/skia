// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0e1db8c86678c004e504f47641b44b17
REG_FIDDLE(IRect_023, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect;
    rect.setXYWH(5, 35, -15, 25);
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect.sort();
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
