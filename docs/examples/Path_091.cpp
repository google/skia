// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d9ecd58081b5bc77a157636fcb345dc6
REG_FIDDLE(Path_091, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRRect rrect;
    SkVector radii[] = {{50, 50}, {0, 0}, {0, 0}, {50, 50}};
    rrect.setRectRadii({10, 10, 110, 110}, radii);
    SkPath path;
    SkMatrix rotate90;
    rotate90.setRotate(90, 128, 128);
    for (int i = 0; i < 4; ++i) {
        path.addRRect(rrect);
        path.transform(rotate90);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
