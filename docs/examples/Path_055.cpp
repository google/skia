// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1c1f4cdef1c572c9aa8fdf3e461191d0
REG_FIDDLE(Path_055, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.moveTo(128, 20);
    path.rQuadTo(-6, 10, -7, 10);
    for (int i = 1; i < 32; i += 4) {
       path.rQuadTo(10 + i, 10 + i, 10 + i * 4, 10);
       path.rQuadTo(-10 - i, 10 + i, -10 - (i + 2) * 4, 10);
    }
    path.quadTo(92, 220, 128, 215);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
