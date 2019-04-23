#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6fa5e8f9513b3225e106778592e27e94
REG_FIDDLE(Path_setLastPt_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath path, path2;
    paint.getTextPath("A", 1, 60, 100, &path);
    paint.getTextPath("Z", 1, 60, 100, &path2);
    SkPoint pt, pt2;
    path.getLastPt(&pt);
    path2.getLastPt(&pt2);
    path.setLastPt(pt2);
    path2.setLastPt(pt);
    canvas->drawPath(path, paint);
    canvas->drawPath(path2, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
