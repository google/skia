#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=542c5afaea5f57baa11d0561dd402e18
REG_FIDDLE(Path_setLastPt, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath path;
    paint.getTextPath("@", 1, 60, 100, &path);
    path.setLastPt(20, 120);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
