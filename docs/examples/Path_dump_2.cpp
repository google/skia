// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_dump_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, copy;
    path.lineTo(6.f / 7, 2.f / 3);
    path.dump();
    copy.setFillType(SkPathFillType::kWinding);
    copy.moveTo(0, 0);
    copy.lineTo(0.857143f, 0.666667f);
    SkDebugf("path is " "%s" "equal to copy\n", path == copy ? "" : "not ");
}
}  // END FIDDLE
