// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=24736f685f265cf533f1700c042db353
REG_FIDDLE(Path_089, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    for (auto xradius : { 0, 7, 13, 20 } ) {
        for (auto yradius : { 0, 9, 18, 40 } ) {
            SkPath path;
            path.addRoundRect({10, 10, 36, 46}, xradius, yradius);
            paint.setColor(path.isRect(nullptr) ? SK_ColorRED : path.isOval(nullptr) ?
                           SK_ColorBLUE : path.isConvex() ? SK_ColorGRAY : SK_ColorGREEN);
            canvas->drawPath(path, paint);
            canvas->translate(64, 0);
        }
        canvas->translate(-256, 64);
    }
}
}  // END FIDDLE
