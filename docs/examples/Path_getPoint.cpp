// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_getPoint, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder().lineTo(20, 20).detach();
    path = path.makeOffset(-10, -10);
    for (int i= 0; i < path.countPoints(); ++i) {
         SkDebugf("point %d: (%1.8g,%1.8g)\n", i, path.getPoint(i).fX, path.getPoint(i).fY);
    }
}
}  // END FIDDLE
