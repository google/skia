// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=2049ff5141f0c80aac497618622b28af
REG_FIDDLE(Path_setIsVolatile, 50, 50, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path;
    path.setIsVolatile(true);
    path.lineTo(40, 40);
    canvas->drawPath(path, paint);
    path.rewind();
    path.moveTo(0, 40);
    path.lineTo(40, 0);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
