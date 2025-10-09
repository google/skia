// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_getPoints, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path, SkPoint* points, int max) -> void {
        int count = path.getPoints({points, max});
         SkDebugf("%s point count: %d  ", prefix, count);
         for (int i = 0; i < std::min(count, max) && points; ++i) {
             SkDebugf("(%1.8g,%1.8g) ", points[i].fX, points[i].fY);
         }
         SkDebugf("\n");
    };
    SkPath path = SkPathBuilder()
                  .lineTo(20, 20)
                  .lineTo(-10, -10)
                  .detach();
    SkPoint points[3];
    debugster("no points",  path, nullptr, 0);
    debugster("zero max",  path, points, 0);
    debugster("too small",  path, points, 2);
    debugster("just right",  path, points, path.countPoints());
}
}  // END FIDDLE
