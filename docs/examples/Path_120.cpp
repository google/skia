// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6c9688008cea8937ad5cc188b38ecf16
REG_FIDDLE(Path_120, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, SkPath::Iter& iter) -> void {
        SkDebugf("%s:\n", prefix);
        const char* verbStr[] =  { "Move", "Line", "Quad", "Conic", "Cubic", "Close", "Done" };
        const int pointCount[] = {     1 ,     2 ,     3 ,      3 ,      4 ,      1 ,     0  };
        SkPath::Verb verb;
        do {
           SkPoint points[4];
           verb = iter.next(points);
           SkDebugf("k%s_Verb ", verbStr[(int) verb]);
           for (int i = 0; i < pointCount[(int) verb]; ++i) {
                SkDebugf("{%g, %g}, ", points[i].fX, points[i].fY);
           }
           if (SkPath::kConic_Verb == verb) {
               SkDebugf("weight = %g", iter.conicWeight());
           }
           SkDebugf("\n");
        } while (SkPath::kDone_Verb != verb);
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
