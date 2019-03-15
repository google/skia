// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=87176fc60914cbca9c6a20998a033c24
REG_FIDDLE(Rect_039, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 50, 20, 60 };
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect = rect.makeOutset(15, 32);
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
