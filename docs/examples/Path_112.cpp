// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=72a92fe058e8b3be6c8a30fad7fd1266
REG_FIDDLE(Path_112, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, copy;
    path.lineTo(6.f / 7, 2.f / 3);
    path.dumpHex();
    copy.setFillType(SkPath::kWinding_FillType);
    copy.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    copy.lineTo(SkBits2Float(0x3f5b6db7), SkBits2Float(0x3f2aaaab));  // 0.857143f, 0.666667f
    SkDebugf("path is " "%s" "equal to copy\n", path == copy ? "" : "not ");
}
}  // END FIDDLE
