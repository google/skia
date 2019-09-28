// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a61e5758574e28190ec4ed8c4ae7e7fa
REG_FIDDLE(Path_SegmentMask, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.conicTo(10, 10, 20, 30, 1);
    SkDebugf("Path kConic_SegmentMask is %s\n", path.getSegmentMasks() &
          SkPath::kConic_SegmentMask ? "set" : "clear");
    SkDebugf("Path kQuad_SegmentMask is %s\n", path.getSegmentMasks() &
          SkPath::kQuad_SegmentMask ? "set" : "clear");
}
}  // END FIDDLE
