// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=13044dbf68885c0f15322c0633b633a3
REG_FIDDLE(Path_Iter_const_SkPath, 256, 256, true, 0) {
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
    SkPath::Iter openIter(path, false);
    debugster("open", openIter);
    SkPath::Iter closedIter(path, true);
    debugster("closed", closedIter);
}
}  // END FIDDLE
