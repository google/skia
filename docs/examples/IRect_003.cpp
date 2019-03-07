// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ec1473b700c594f2df9749a12a06b89b
REG_FIDDLE(IRect_003, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = SkIRect::MakeLTRB(5, 35, 15, 25);
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect.sort();
    SkDebugf("rect: %d, %d, %d, %d  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
