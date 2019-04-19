#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7f27c93472aa99a7542fb3493076f072
REG_FIDDLE(Paint_getPosTextPath, 256, 85, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(80);
    SkPath path, path2;
    SkPoint pos[] = {{20, 60}, {30, 70}, {40, 80}};
    paint.getPosTextPath("ABC", 3, pos, &path);
    Simplify(path, &path);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
