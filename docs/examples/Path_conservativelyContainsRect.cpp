// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=41638d13e40fa449ece354dde5fb1941
REG_FIDDLE(Path_conservativelyContainsRect, 256, 140, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.addRoundRect({10, 20, 54, 120}, 10, 20);
    SkRect tests[] = {
      { 10, 40, 54, 80 },
      { 25, 20, 39, 120 },
      { 15, 25, 49, 115 },
      { 13, 27, 51, 113 },
    };
    for (unsigned i = 0; i < SK_ARRAY_COUNT(tests); ++i) {
      SkPaint paint;
      paint.setColor(SK_ColorRED);
      canvas->drawPath(path, paint);
      bool rectInPath = path.conservativelyContainsRect(tests[i]);
      paint.setColor(rectInPath ? SK_ColorBLUE : SK_ColorBLACK);
      canvas->drawRect(tests[i], paint);
      canvas->translate(64, 0);
    }
}
}  // END FIDDLE
