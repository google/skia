// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Conic_Weight_c, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* verbNames[] = { "move", "line", "quad", "conic", "cubic", "close", "done" };
    SkPath path = SkPathBuilder().conicTo(20, 0, 20, 20, SK_ScalarInfinity).detach();
    SkPath::Iter iter(path, false);
    while (auto rec = iter.next()) {
       SkDebugf("%s ", verbNames[(int)rec->fVerb]);
        for (SkPoint p : rec->fPoints) {
            SkDebugf("{%g, %g}, ", p.fX, p.fY);
       }
       if (SkPathVerb::kConic == rec->fVerb) {
           SkDebugf("weight = %g", rec->conicWeight());
       }
       SkDebugf("\n");
    }
}
}  // END FIDDLE
