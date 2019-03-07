// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6b97099acdae80b16df0c4241f593991
REG_FIDDLE(Path_032, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint points[] = {{1, 0}, {0, 0}, {0, 0}, {0, 0}};
    SkScalar step = 1;
    SkScalar prior, length = 0, degenerate = 0;
    do {
        prior = points[0].fX;
        step /= 2;
        if (SkPath::IsCubicDegenerate(points[0], points[1], points[2], points[3], false)) {
            degenerate = prior;
            points[0].fX += step;
        } else {
            length = prior;
            points[0].fX -= step;
        }
    } while (prior != points[0].fX);
    SkDebugf("%1.8g is degenerate\n", degenerate);
    SkDebugf("%1.8g is length\n", length);
}
}  // END FIDDLE
