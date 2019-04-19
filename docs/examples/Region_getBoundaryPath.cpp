// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6631d36406efa3b3e27960c876421a7f
REG_FIDDLE(Region_getBoundaryPath, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    region.setRect({10, 20, 90, 60});
    region.op({30, 40, 60, 80}, SkRegion::kXOR_Op);
    canvas->drawRegion(region, SkPaint());
    SkPath path;
    region.getBoundaryPath(&path);
    path.offset(100, 0);
    canvas->drawPath(path, SkPaint());
}
}  // END FIDDLE
