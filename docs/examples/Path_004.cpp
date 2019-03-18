// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=01ad6be9b7d15a2217daea273eb3d466
REG_FIDDLE(Path_destructor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath* path = new SkPath();
    path->lineTo(20, 20);
    SkPath path2(*path);
    delete path;
    SkDebugf("path2 is " "%s" "empty", path2.isEmpty() ? "" : "not ");
}
}  // END FIDDLE
