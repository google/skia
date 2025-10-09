// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_Iter_const_SkPath, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, SkPath::Iter& iter) -> void {
        SkDebugf("%s:\n", prefix);
        const char* verbStr[] =  { "Move", "Line", "Quad", "Conic", "Cubic", "Close", "Done" };
        while (auto rec = iter.next()) {
           SkDebugf("k%s_Verb ", verbStr[(int)rec->fVerb]);
            for (SkPoint p : rec->fPoints) {
                SkDebugf("{%g, %g}, ", p.fX, p.fY);
           }
           if (SkPathVerb::kConic == rec->fVerb) {
               SkDebugf("weight = %g", rec->conicWeight());
           }
           SkDebugf("\n");
        }
        SkDebugf("\n");
    };
    SkPath path = SkPathBuilder().quadTo(10, 20, 30, 40).detach();
    SkPath::Iter openIter(path, false);
    debugster("open", openIter);
    SkPath::Iter closedIter(path, true);
    debugster("closed", closedIter);
}
}  // END FIDDLE
