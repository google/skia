// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_isInterpolatable, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, path2;
    path = SkPathBuilder()
           .moveTo(20, 20)
           .lineTo(40, 40)
           .lineTo(20, 20)
           .lineTo(40, 40)
           .close()
           .detach();
    path2 = SkPath::Rect({20, 20, 40, 40});
    SkDebugf("paths are " "%s" "interpolatable", path.isInterpolatable(path2) ? "" : "not ");
}
}  // END FIDDLE
