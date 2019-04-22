// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d08d1e7dafcad4342d1619fdbb2f5781
REG_FIDDLE(Point_set, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pt1, pt2 = { SK_ScalarPI, SK_ScalarSqrt2 };
    pt1.set(SK_ScalarPI, SK_ScalarSqrt2);
    SkDebugf("pt1 %c= pt2\n", pt1 == pt2 ? '=' : '!');
}
}  // END FIDDLE
