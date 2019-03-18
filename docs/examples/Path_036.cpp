// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=42885f1df13de109adccc5d531f62111
REG_FIDDLE(Path_getPoint, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.lineTo(20, 20);
    path.offset(-10, -10);
    for (int i= 0; i < path.countPoints(); ++i) {
         SkDebugf("point %d: (%1.8g,%1.8g)\n", i, path.getPoint(i).fX, path.getPoint(i).fY);
    }
}
}  // END FIDDLE
