// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d266e70977847001f7c42f8a2513bee7
REG_FIDDLE(Point_000, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pt1 = {45, 66};
    SkPoint pt2 = SkPoint::Make(45, 66);
    SkVector v1 = {45, 66};
    SkVector v2 = SkPoint::Make(45, 66);
    SkDebugf("all %s" "equal\n", pt1 == pt2 && pt2 == v1 && v1 == v2 ? "" : "not ");
}
}  // END FIDDLE
