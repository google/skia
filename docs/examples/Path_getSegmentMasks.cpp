// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=657a3f3e11acafea92b84d6bb0c13633
REG_FIDDLE(Path_getSegmentMasks, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.quadTo(20, 30, 40, 50);
    path.close();
    const char* masks[] = { "line", "quad", "conic", "cubic" };
    int index = 0;
    for (auto mask : { SkPath::kLine_SegmentMask, SkPath::kQuad_SegmentMask,
            SkPath::kConic_SegmentMask, SkPath::kCubic_SegmentMask } ) {
        if (mask & path.getSegmentMasks()) {
           SkDebugf("mask %s set\n", masks[index]);
        }
        ++index;
    }
}
}  // END FIDDLE
