// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2aadded3d20dfef34d1c8abe28c7bc8d
REG_FIDDLE(Conic_Weight_a, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* verbNames[] = { "move", "line", "quad", "conic", "cubic", "close", "done" };
    const int pointCount[]  = {     1 ,     2 ,     3 ,      3 ,      4 ,      1 ,     0  };
    SkPath path;
    path.conicTo(20, 30, 50, 60, 1);
    SkPath::Iter iter(path, false);
    SkPath::Verb verb;
    do {
       SkPoint points[4];
       verb = iter.next(points);
       SkDebugf("%s ", verbNames[(int) verb]);
       for (int i = 0; i < pointCount[(int) verb]; ++i) {
            SkDebugf("{%g, %g}, ", points[i].fX, points[i].fY);
       }
       if (SkPath::kConic_Verb == verb) {
           SkDebugf("weight = %g", iter.conicWeight());
       }
       SkDebugf("\n");
    } while (SkPath::kDone_Verb != verb);
}
}  // END FIDDLE
