// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a8f36f2fa90003e3691fd0da0bb0c243
REG_FIDDLE(Path_getConvexity, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s path convexity is %s\n", prefix,
                SkPathConvexityType::kUnknown == path.getConvexityType() ? "unknown" :
                SkPathConvexityType::kConvex == path.getConvexityType() ? "convex" : "concave"); };
    SkPath path;
    debugster("initial", path);
    path.lineTo(50, 0);
    debugster("first line", path);
    path.lineTo(50, 50);
    debugster("second line", path);
    path.lineTo(100, 50);
    debugster("third line", path);
}
}  // END FIDDLE
