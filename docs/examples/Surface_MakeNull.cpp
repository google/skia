// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=99a54b814ccab7d2b1143c88581649ff
REG_FIDDLE(Surface_MakeNull, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("SkSurfaces::Null(0, 0) %c= nullptr\n", SkSurfaces::Null(0, 0) == nullptr ?
             '=' : '!');
    const int w = 37;
    const int h = 1000;
    auto surf = SkSurfaces::Null(w, h);
    auto nullCanvas = surf->getCanvas();
    nullCanvas->drawPaint(SkPaint());   // does not crash, nothing draws
    SkDebugf("surf->makeImageSnapshot() %c= nullptr\n", surf->makeImageSnapshot() == nullptr ?
            '=' : '!');
}
}  // END FIDDLE
