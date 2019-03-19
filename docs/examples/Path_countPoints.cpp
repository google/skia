// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bca6379ccef62cb081b10db7381deb27
REG_FIDDLE(Path_countPoints, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
         SkDebugf("%s point count: %d\n", prefix, path.countPoints());
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
