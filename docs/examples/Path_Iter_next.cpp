// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=00ae8984856486bdb626d0ed6587855a
REG_FIDDLE(Path_Iter_next, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(10, 10);
    path.moveTo(20, 20);
    path.quadTo(10, 20, 30, 40);
    path.moveTo(1, 1);
    path.close();
    path.moveTo(30, 30);
    path.lineTo(30.00001f, 30);

    SkPath::Iter iter(path, false);
    const char* verbStr[] =  { "Move", "Line", "Quad", "Conic", "Cubic", "Close", "Done" };
    const int pointCount[] = {     1 ,     2 ,     3 ,      3 ,      4 ,      1 ,     0  };
    SkPath::Verb verb;
    do {
        SkPoint points[4];
        verb = iter.next(points);
        SkDebugf("k%s_Verb ", verbStr[(int) verb]);
        for (int i = 0; i < pointCount[(int) verb]; ++i) {
            SkDebugf("{%1.8g, %1.8g}, ", points[i].fX, points[i].fY);
        }
        SkDebugf("\n");
    } while (SkPath::kDone_Verb != verb);
    SkDebugf("\n");
}
}  // END FIDDLE
