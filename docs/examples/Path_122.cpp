// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7cdea37741d50f0594c6244eb07fd175
REG_FIDDLE(Path_122, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPath path;
   path.conicTo(1, 2, 3, 4, .5f);
   SkPath::Iter iter(path, false);
   SkPoint p[4];
   SkDebugf("first verb is " "%s" "move\n", SkPath::kMove_Verb == iter.next(p) ? "" : "not ");
   SkDebugf("next verb is " "%s" "conic\n", SkPath::kConic_Verb == iter.next(p) ? "" : "not ");
   SkDebugf("conic points: {%g,%g}, {%g,%g}, {%g,%g}\n", p[0].fX, p[0].fY, p[1].fX, p[1].fY,
                p[2].fX, p[2].fY);
   SkDebugf("conic weight: %g\n", iter.conicWeight());
}
}  // END FIDDLE
