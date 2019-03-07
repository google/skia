// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e5cf5159525bd3140f288a95fe641fae
REG_FIDDLE(IPoint_000, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint pt1 = {45, 66};
    SkIPoint pt2 = SkIPoint::Make(45, 66);
    SkDebugf("pt1 %c= pt2\n", pt1 == pt2 ? '=' : '!');
}
}  // END FIDDLE
