// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c81fc7dfaf785c3fb77209c7f2ebe5b8
REG_FIDDLE(Path_isInterpolatable, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, path2;
    path.moveTo(20, 20);
    path.lineTo(40, 40);
    path.lineTo(20, 20);
    path.lineTo(40, 40);
    path.close();
    path2.addRect({20, 20, 40, 40});
    SkDebugf("paths are " "%s" "interpolatable", path.isInterpolatable(path2) ? "" : "not ");
}
}  // END FIDDLE
