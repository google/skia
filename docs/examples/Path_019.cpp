// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=875e32b4b1cb48d739325705fc0fa42c
REG_FIDDLE(Path_019, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s path convexity is %s\n", prefix,
                SkPath::kUnknown_Convexity == path.getConvexity() ? "unknown" :
                SkPath::kConvex_Convexity == path.getConvexity() ? "convex" : "concave"); };
        SkPoint quad[] = {{70, 70}, {20, 20}, {120, 20}, {120, 120}};
        SkPath path;
        path.addPoly(quad, SK_ARRAY_COUNT(quad), true);
        debugster("initial", path);
        path.setConvexity(SkPath::kConcave_Convexity);
        debugster("after forcing concave", path);
        path.setConvexity(SkPath::kUnknown_Convexity);
        debugster("after forcing unknown", path);
}
}  // END FIDDLE
