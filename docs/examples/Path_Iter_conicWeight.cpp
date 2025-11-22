// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_Iter_conicWeight, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPath path = SkPathBuilder().conicTo(1, 2, 3, 4, .5f).detach();
   SkPath::Iter iter(path, false);
   SkDebugf("first verb is " "%s" "move\n", SkPathVerb::kMove == iter.next()->fVerb ? "" : "not ");
   auto rec = iter.next();
   SkDebugf("next verb is " "%s" "conic\n", SkPathVerb::kConic == rec->fVerb ? "" : "not ");
   SkSpan<const SkPoint> p = rec->fPoints;
   SkDebugf("conic points: {%g,%g}, {%g,%g}, {%g,%g}\n", p[0].fX, p[0].fY, p[1].fX, p[1].fY,
                p[2].fX, p[2].fY);
   SkDebugf("conic weight: %g\n", rec->conicWeight());
}
}  // END FIDDLE
