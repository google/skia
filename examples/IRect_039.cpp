// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(IRect_039, 256, 256, true, 0);
// HASH=ea233f5d5d1ae0e76fc6f2eb371c927a
void draw(SkCanvas* canvas) {
    SkIRect leftRect =  { 10, 40, 50, 80 };
    SkIRect rightRect = { 30, 60, 70, 90 };
    SkDebugf("%s intersection: ", leftRect.intersect(rightRect) ? "" : "no ");
    SkDebugf("%d, %d, %d, %d\n", leftRect.left(), leftRect.top(),
                                 leftRect.right(), leftRect.bottom());
}
}
