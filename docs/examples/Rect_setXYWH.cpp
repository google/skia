// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=373cce4c61b9da0384b735b838765163
REG_FIDDLE(Rect_setXYWH, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect;
    rect.setXYWH(5, 35, -15, 25);
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect.sort();
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
