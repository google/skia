// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5188d77585715db30bef228f2dfbcccd
REG_FIDDLE(Path_offset_2, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(20, 20);
    path.lineTo(20, 40);
    path.lineTo(40, 20);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 10; i++) {
        canvas->drawPath(path, paint);
        path.offset(20, 0);
    }
}
}  // END FIDDLE
