// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=41001546a7f7927d08e5a818bcc304f5
REG_FIDDLE(Path_lineTo_2, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    SkVector oxo[] = {{25, 25}, {35, 35}, {25, 35}, {35, 25},
                      {40, 20}, {40, 80}, {60, 20}, {60, 80},
                      {20, 40}, {80, 40}, {20, 60}, {80, 60}};
    for (unsigned i = 0; i < SK_ARRAY_COUNT(oxo); i += 2) {
        path.moveTo(oxo[i]);
        path.lineTo(oxo[i + 1]);
    }
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
