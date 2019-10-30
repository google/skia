// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=814efa7d7f4ae52dfc861a937c1b5c25
REG_FIDDLE(Region_Iterator_done, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    SkRegion::Iterator iter(region);
    SkDebugf("done=%s\n", iter.done() ? "true" : "false");
    region.setRect({1, 2, 3, 4});
    iter.rewind();
    SkDebugf("done=%s\n", iter.done() ? "true" : "false");
}
}  // END FIDDLE
