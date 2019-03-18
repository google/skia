// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9cf5122475624e4cf39f06c698f80b1a
REG_FIDDLE(Path_addArc, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    for (auto start : { 0, 90, 135, 180, 270 } ) {
        for (auto sweep : { -450.f, -180.f, -90.f, 90.f, 180.f, 360.1f } ) {
            SkPath path;
            path.addArc({10, 10, 35, 45}, start, sweep);
            canvas->drawPath(path, paint);
            canvas->translate(252 / 6, 0);
        }
        canvas->translate(-252, 255 / 5);
    }
}
}  // END FIDDLE
