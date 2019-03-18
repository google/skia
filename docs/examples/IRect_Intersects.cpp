// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0c67cf8981389efc7108369fb9b7976b
REG_FIDDLE(IRect_Intersects, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("%s intersection", SkIRect::Intersects({10, 40, 50, 80}, {30, 60, 70, 90}) ? "" : "no ");
}
}  // END FIDDLE
