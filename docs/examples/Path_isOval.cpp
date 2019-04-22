// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a51256952b183ee0f7004f2c87cbbf5b
REG_FIDDLE(Path_isOval, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath path;
    path.addOval({20, 20, 220, 220});
    SkRect bounds;
    if (path.isOval(&bounds)) {
        paint.setColor(0xFF9FBFFF);
        canvas->drawRect(bounds, paint);
    }
    paint.setColor(0x3f000000);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
