// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=59a6e7d202ac17ab80ec21b233e51f59
REG_FIDDLE(Rect_toQuad, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = {1, 2, 3, 4};
    SkPoint corners[4];
    rect.toQuad(corners);
    SkDebugf("rect: {%g, %g, %g, %g}\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    SkDebugf("corners:");
    for (auto corner : corners) {
        SkDebugf(" {%g, %g}", corner.fX, corner.fY);
    }
    SkDebugf("\n");
}
}  // END FIDDLE
