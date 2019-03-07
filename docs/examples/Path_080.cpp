// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=81a2aac1b8f0ff3d4c8d35ccb9149b16
REG_FIDDLE(Path_080, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkRect rect;
        SkPath::Direction direction;
        bool isClosed;
        path.isRect(&rect, &isClosed, &direction) ?
                SkDebugf("%s is rect (%g, %g, %g, %g); is %s" "closed; direction %s\n", prefix,
                         rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, isClosed ? "" : "not ",
                         SkPath::kCW_Direction == direction ? "CW" : "CCW") :
                SkDebugf("%s is not rect\n", prefix);
    };
    SkPath path;
    debugster("empty", path);
    path.addRect({10, 20, 30, 40});
    debugster("addRect", path);
    path.moveTo(60, 70);
    debugster("moveTo", path);
    path.lineTo(60, 70);
    debugster("lineTo", path);
    path.reset();
    const SkPoint pts[] = { {0, 0}, {0, 80}, {80, 80}, {80, 0}, {40, 0}, {20, 0} };
    path.addPoly(pts, SK_ARRAY_COUNT(pts), false);
    debugster("addPoly", path);
}
}  // END FIDDLE
