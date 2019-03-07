// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cb8d37990f6e7df3bcc85e7240c81274
REG_FIDDLE(Path_046, 128, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPoint data[][3] = {{{30,40},{60,60},{90,30}}, {{30,120},{60,100},{90,120}},
                         {{60,100},{60,40},{70,30}}, {{60,40},{50,20},{70,30}}};
    SkPath path;
    for (unsigned i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        path.moveTo(data[i][0]);
        path.lineTo(data[i][1]);
        path.lineTo(data[i][2]);
    }
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
