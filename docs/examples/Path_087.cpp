// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bd5286cb9a5e5c32cd980f72b8f400fb
REG_FIDDLE(Path_087, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    for (int size = 10; size < 300; size += 20) {
        SkPath path;
        path.addCircle(128, 128, size, SkPath::kCW_Direction);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
