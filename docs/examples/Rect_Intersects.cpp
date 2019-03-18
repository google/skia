// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=795061764b10c9e05efb466c9cb60644
REG_FIDDLE(Rect_Intersects, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("%s intersection", SkRect::Intersects({10, 40, 50, 80}, {30, 60, 70, 90}) ? "" : "no ");
}
}  // END FIDDLE
