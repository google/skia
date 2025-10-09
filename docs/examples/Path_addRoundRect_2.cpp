// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_addRoundRect_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkVector radii[] = { {80, 100}, {0, 0}, {40, 60}, {0, 0} };
    SkRRect rr;
    rr.setRectRadii({10, 10, 110, 110}, radii);

    SkPathBuilder path;
    SkMatrix rotate90;
    rotate90.setRotate(90, 128, 128);
    for (int i = 0; i < 4; ++i) {
        path.addRRect(rr);
        path.transform(rotate90);
    }
    canvas->drawPath(path.detach(), paint);
}
}  // END FIDDLE
