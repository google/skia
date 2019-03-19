// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a2b255a7dac1926cc3a247d318d63c62
REG_FIDDLE(Path_IsQuadDegenerate, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const SkPath& path, bool exact) -> void {
        SkDebugf("quad (%1.8g,%1.8g), (%1.8g,%1.8g), (%1.8g,%1.8g) is %s" "degenerate, %s\n",
            path.getPoint(0).fX, path.getPoint(0).fY, path.getPoint(1).fX,
            path.getPoint(1).fY, path.getPoint(2).fX, path.getPoint(2).fY,
            SkPath::IsQuadDegenerate(path.getPoint(0), path.getPoint(1), path.getPoint(2), exact) ?
            "" : "not ", exact ? "exactly" : "nearly");
    };
    SkPath path, offset;
    path.moveTo({100, 100});
    path.quadTo({100.00001f, 100.00001f}, {100.00002f, 100.00002f});
    offset.addPath(path, 1000, 1000);
    for (bool exact : { false, true } ) {
        debugster(path, exact);
        debugster(offset, exact);
    }
}
}  // END FIDDLE
