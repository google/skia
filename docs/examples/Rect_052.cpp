// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ca37b4231b21eb8296cb19ba9e0c781b
REG_FIDDLE(Rect_intersects_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 40, 50, 80 };
    SkDebugf("%s intersection", rect.intersects({30, 60, 70, 90}) ? "" : "no ");
}
}  // END FIDDLE
