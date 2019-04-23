// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=afa9c6b4d05bb669db07fe0b7b97e6aa
REG_FIDDLE(Rect_join, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 20, 15, 25};
    rect.join(50, 60, 55, 65);
    SkDebugf("join: %g, %g, %g, %g\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}  // END FIDDLE
