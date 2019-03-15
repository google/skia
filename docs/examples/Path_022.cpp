// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2aa939b90d96aff436b145a96305132c
REG_FIDDLE(Path_022, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath path;
    path.addRRect(SkRRect::MakeRectXY({20, 20, 220, 220}, 30, 50));
    SkRRect rrect;
    if (path.isRRect(&rrect)) {
        const SkRect& bounds = rrect.rect();
        paint.setColor(0xFF9FBFFF);
        canvas->drawRect(bounds, paint);
    }
    paint.setColor(0x3f000000);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
