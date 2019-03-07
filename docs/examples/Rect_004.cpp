// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=158b8dd9d02d65a5ae5ab7d1595a5b4c
REG_FIDDLE(Rect_004, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = SkRect::MakeLTRB(5, 35, 15, 25);
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
    rect.sort();
    SkDebugf("rect: %g, %g, %g, %g  isEmpty: %s\n", rect.left(), rect.top(), rect.right(),
              rect.bottom(), rect.isEmpty() ? "true" : "false");
}
}  // END FIDDLE
