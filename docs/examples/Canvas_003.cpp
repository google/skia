// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ce6a5ef2df447970b4453489d9d67930
REG_FIDDLE(Canvas_003, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    SkCanvas canvas(10, 20);  // 10 units wide, 20 units high
    canvas.clipRect(SkRect::MakeXYWH(30, 40, 5, 10));  // clip is outside canvas' device
    SkDebugf("canvas %s empty\n", canvas.getDeviceClipBounds().isEmpty() ? "is" : "is not");
}
}  // END FIDDLE
