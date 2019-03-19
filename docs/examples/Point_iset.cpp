// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0d9e8ed734981b5b113f22c7bfde5357
REG_FIDDLE(Point_iset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pt1, pt2 = { SK_MinS16, SK_MaxS16 };
    pt1.iset(SK_MinS16, SK_MaxS16);
    SkDebugf("pt1 %c= pt2\n", pt1 == pt2 ? '=' : '!');
}
}  // END FIDDLE
