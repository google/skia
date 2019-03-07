// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Region_026, 256, 256, true, 0);
// HASH=651632582d385d2531e7aa551c31e331
void draw(SkCanvas* canvas) {
    SkRegion region({1, 2, 3, 4});
    region.op({2, 3, 4, 5}, SkRegion::kUnion_Op);
    auto r = region.getBounds();
    SkDebugf("bounds: {%d,%d,%d,%d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}
