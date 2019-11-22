// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3f76a1007416181a4848c1a87fc81dbd
REG_FIDDLE(Path_rArcTo, 256, 108, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath path;
    const SkPoint starts[] = {{20, 20}, {120, 20}, {70, 60}};
    for (auto start : starts) {
        path.moveTo(start.fX, start.fY);
        path.rArcTo(20, 20, 0, SkPath::kSmall_ArcSize, SkPath::kCCW_Direction, 60, 0);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
