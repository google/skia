// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
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
    while (auto rec = iter.next()) {
        SkDebugf("k%s_Verb ", verbStr[(int)rec->fVerb]);
        for (SkPoint p : rec->fPoints) {
            SkDebugf("{%1.8g, %1.8g}, ", p.fX, p.fY);
        }
        SkDebugf("\n");
    }
    SkDebugf("\n");
}
}  // END FIDDLE
