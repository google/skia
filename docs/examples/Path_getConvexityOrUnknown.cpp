// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=111c59e9afadb940ab8f41bdc25378a4
REG_FIDDLE(Path_getConvexityOrUnknown, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s path convexity is %s\n", prefix,
            SkPath::kUnknown_Convexity == path.getConvexityOrUnknown() ? "unknown" :
            SkPath::kConvex_Convexity == path.getConvexityOrUnknown() ? "convex" : "concave"); };
    SkPath path;
    debugster("initial", path);
    path.lineTo(50, 0);
    debugster("first line", path);
    path.getConvexity();
    path.lineTo(50, 50);
    debugster("second line", path);
    path.lineTo(100, 50);
    path.getConvexity();
    debugster("third line", path);
}
}  // END FIDDLE
