// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c43d70606b4ee464d2befbcf448c5e73
REG_FIDDLE(Path_090, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkScalar radii[] = { 80, 100, 0, 0, 40, 60, 0, 0 };
    SkPath path;
    SkMatrix rotate90;
    rotate90.setRotate(90, 128, 128);
    for (int i = 0; i < 4; ++i) {
        path.addRoundRect({10, 10, 110, 110}, radii);
        path.transform(rotate90);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
