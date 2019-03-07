// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5d0b12e0ef6f1c181dddded4274230ca
REG_FIDDLE(Rect_048, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect leftRect =  { 10, 40, 50, 80 };
    SkRect rightRect = { 30, 60, 70, 90 };
    SkDebugf("%s intersection: ", leftRect.intersect(rightRect) ? "" : "no ");
    SkDebugf("%g, %g, %g, %g\n", leftRect.left(), leftRect.top(),
                                 leftRect.right(), leftRect.bottom());
}
}  // END FIDDLE
