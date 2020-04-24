#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=7c9e6a399f898d68026c1f0865e6f73e
REG_FIDDLE(Paint_getTextPath, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(80);
    SkPath path, path2;
    paint.getTextPath("ABC", 3, 20, 80, &path);
    path.offset(20, 20, &path2);
    Op(path, path2, SkPathOp::kDifference_SkPathOp, &path);
    path.addPath(path2);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
