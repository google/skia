// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=84101d341e934a535a41ad6cf42218ce
REG_FIDDLE(Path_moveTo, 140, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 20, 20, 120, 80 };
    SkPath path;
    path.addRect(rect);
    path.moveTo(rect.fLeft, rect.fTop);
    path.lineTo(rect.fRight, rect.fBottom);
    path.moveTo(rect.fLeft, rect.fBottom);
    path.lineTo(rect.fRight, rect.fTop);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
