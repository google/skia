// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_Iter_setPath, 256, 256, true, 0) {
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
    SkPath path;
    path.quadTo(10, 20, 30, 40);
    SkPath::Iter iter(path, false);
    debugster("quad open", iter);
    SkPath path2;
    path2.conicTo(1, 2, 3, 4, .5f);
    iter.setPath(path2, true);
    debugster("conic closed", iter);
}
}  // END FIDDLE
