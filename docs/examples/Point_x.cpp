// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9f3fe446b800ae1d940785d438634941
REG_FIDDLE(Point_x, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pt1 = {45, 66};
    SkDebugf("pt1.fX %c= pt1.x()\n", pt1.fX == pt1.x() ? '=' : '!');
}
}  // END FIDDLE
