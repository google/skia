// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=22d25e03b19d5bae92118877e462361b
REG_FIDDLE(Path_060, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkRect oval = {0, 20, 120, 140};
    SkPath path;
    for (int i = 0; i < 4; ++i) {
        path.moveTo(oval.centerX(), oval.fTop);
        path.arcTo(oval, -90, 90 - 20 * i, false);
        oval.inset(15, 15);
    }
    path.offset(100, 0);
    SkScalar conicWeights[] = { 0.707107f, 0.819152f, 0.906308f, 0.965926f };
    SkPoint conicPts[][3] = { { {40, 20}, {100, 20}, {100, 80} },
                              { {40, 35}, {71.509f, 35}, {82.286f, 64.6091f} },
                              { {40, 50}, {53.9892f, 50}, {62.981f, 60.7164f} },
                              { {40, 65}, {44.0192f, 65}, {47.5f, 67.0096f} } };
    for (int i = 0; i < 4; ++i) {
         path.moveTo(conicPts[i][0]);
         path.conicTo(conicPts[i][1], conicPts[i][2], conicWeights[i]);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
