// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3e476378e3e0550ab134bbaf61112d98
REG_FIDDLE(Path_063, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path;
    path.moveTo(0, -10);
    for (int i = 0; i < 128; i += 16) {
        SkScalar c = i * 0.5f;
        path.cubicTo( 10 + c, -10 - i,  10 + i, -10 - c,  10 + i,       0);
        path.cubicTo( 14 + i,  14 + c,  14 + c,  14 + i,       0,  14 + i);
        path.cubicTo(-18 - c,  18 + i, -18 - i,  18 + c, -18 - i,       0);
        path.cubicTo(-22 - i, -22 - c, -22 - c, -22 - i,       0, -22 - i);
    }
    path.offset(128, 128);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
