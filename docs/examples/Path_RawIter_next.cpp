// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=944a80c7ff8c04e1fecc4aec4a47ea60
REG_FIDDLE(Path_RawIter_next, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(50, 60);
    path.quadTo(10, 20, 30, 40);
    path.close();
    path.lineTo(30, 30);
    path.conicTo(1, 2, 3, 4, .5f);
    path.cubicTo(-1, -2, -3, -4, -5, -6);
    SkPath::RawIter iter(path);
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
        if (SkPath::kConic_Verb == verb) {
            SkDebugf("weight = %g", iter.conicWeight());
        }
        SkDebugf("\n");
    } while (SkPath::kDone_Verb != verb);
}
}  // END FIDDLE
