// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9bc86efda08cbcd9c6f7c5f220294a24
REG_FIDDLE(Path_getPoints, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path, SkPoint* points, int max) -> void {
         int count = path.getPoints(points, max);
         SkDebugf("%s point count: %d  ", prefix, count);
         for (int i = 0; i < SkTMin(count, max) && points; ++i) {
             SkDebugf("(%1.8g,%1.8g) ", points[i].fX, points[i].fY);
         }
         SkDebugf("\n");
    };
    SkPath path;
    path.lineTo(20, 20);
    path.lineTo(-10, -10);
    SkPoint points[3];
    debugster("no points",  path, nullptr, 0);
    debugster("zero max",  path, points, 0);
    debugster("too small",  path, points, 2);
    debugster("just right",  path, points, path.countPoints());
}
}  // END FIDDLE
