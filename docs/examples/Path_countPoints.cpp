// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_countPoints, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
         SkDebugf("%s point count: %d\n", prefix, path.countPoints());
    };
    SkPath path;
    debugster("empty", path);
    path = SkPathBuilder().lineTo(0, 0).detach();
    debugster("zero line", path);
    path = SkPath::Line({10, 10}, {20, 20});
    debugster("line", path);
}
}  // END FIDDLE
