// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1ad07d56e4258e041606d50cad969392
REG_FIDDLE(Path_033, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkPoint line[2];
        if (path.isLine(line)) {
            SkDebugf("%s is line (%1.8g,%1.8g) (%1.8g,%1.8g)\n", prefix,
                 line[0].fX, line[0].fY, line[1].fX, line[1].fY);
        } else {
            SkDebugf("%s is not line\n", prefix);
        }
    };
    SkPath path;
    debugster("empty", path);
    path.lineTo(0, 0);
    debugster("zero line", path);
    path.rewind();
    path.moveTo(10, 10);
    path.lineTo(20, 20);
    debugster("line", path);
    path.moveTo(20, 20);
    debugster("second move", path);
}
}  // END FIDDLE
