#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d8be8b6e59de244e4cbf58ec9554557b
REG_FIDDLE(Path_isConvex, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPoint quad[] = {{70, 70}, {20, 20}, {120, 20}, {120, 120}};
    for (SkScalar x : { 40, 100 } ) {
        SkPath path;
        quad[0].fX = x;
        path.addPoly(quad, SK_ARRAY_COUNT(quad), true);
        path.setConvexity(SkPath::kConvex_Convexity);
        canvas->drawPath(path, paint);
        canvas->drawString(path.isConvex() ? "convex" : "not convex", 30, 100, paint);
        canvas->translate(100, 100);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
