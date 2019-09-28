// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4c962850c2dbea4d2325df469400680e
REG_FIDDLE(Point_y, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pt1 = {45, 66};
    SkDebugf("pt1.fY %c= pt1.y()\n", pt1.fY == pt1.y() ? '=' : '!');
}
}  // END FIDDLE
