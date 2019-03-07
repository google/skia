// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=38e464dba13be11ac21e210fbf3b5afc
REG_FIDDLE(Rect_005, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = SkRect::MakeXYWH(5, 35, -15, 25);
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect.sort();
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
