// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(bugoftheday, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    SkPath path;
    path.moveTo(10, 10);
    path.moveTo(75, 75);
    path.lineTo(150, 75);
    path.lineTo(150, 150);
    path.lineTo(75, 150);
    canvas->drawPath(path, p);
}
}  // END FIDDLE
