// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=165418b5718d79d8f1682a8a0ee32ba0
REG_FIDDLE(IPoint_set, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint pt1, pt2 = { SK_MinS32, SK_MaxS32 };
    pt1.set(SK_MinS32, SK_MaxS32);
    SkDebugf("pt1 %c= pt2\n", pt1 == pt2 ? '=' : '!');
}
}  // END FIDDLE
