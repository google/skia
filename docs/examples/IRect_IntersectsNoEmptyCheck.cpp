// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=dba234d15162fb5b26e1a96529ca6a2a
REG_FIDDLE(IRect_IntersectsNoEmptyCheck, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkDebugf("%s intersection", SkIRect::IntersectsNoEmptyCheck(
            {10, 40, 50, 80}, {30, 60, 70, 90}) ? "" : "no ");
}
}  // END FIDDLE
