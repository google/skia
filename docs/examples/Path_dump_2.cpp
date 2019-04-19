// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=92e0032f85181795d1f8b5a2c8e4e4b7
REG_FIDDLE(Path_dump_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, copy;
    path.lineTo(6.f / 7, 2.f / 3);
    path.dump();
    copy.setFillType(SkPath::kWinding_FillType);
    copy.moveTo(0, 0);
    copy.lineTo(0.857143f, 0.666667f);
    SkDebugf("path is " "%s" "equal to copy\n", path == copy ? "" : "not ");
}
}  // END FIDDLE
