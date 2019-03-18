// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=69f360a0ba8f40c51ef4cd9f972c5893
REG_FIDDLE(Path_RawIter_conicWeight, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPath path;
   path.conicTo(1, 2, 3, 4, .5f);
   SkPath::RawIter iter(path);
   SkPoint p[4];
   SkDebugf("first verb is " "%s" "move\n", SkPath::kMove_Verb == iter.next(p) ? "" : "not ");
   SkDebugf("next verb is " "%s" "conic\n", SkPath::kConic_Verb == iter.next(p) ? "" : "not ");
   SkDebugf("conic points: {%g,%g}, {%g,%g}, {%g,%g}\n", p[0].fX, p[0].fY, p[1].fX, p[1].fY,
                p[2].fX, p[2].fY);
   SkDebugf("conic weight: %g\n", iter.conicWeight());
}
}  // END FIDDLE
