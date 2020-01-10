// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(bug7573, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    canvas->scale(0.196f * 2, 0.196f * 2);
    SkPath path;
    path.moveTo(SkBits2Float(0x40a1a3f7), SkBits2Float(0x41b80159));  // 5.05127f, 23.0007f
    path.lineTo(SkBits2Float(0x42f41a3f), SkBits2Float(0x41d00159));  // 122.051f, 26.0007f
    path.lineTo(SkBits2Float(0x42f3e5c1), SkBits2Float(0x41effea7));  // 121.949f, 29.9993f
    path.lineTo(SkBits2Float(0x409e5c09), SkBits2Float(0x41d7fea7));  // 4.94873f, 26.9993f
    path.lineTo(SkBits2Float(0x40a1a3f7), SkBits2Float(0x41b80159));  // 5.05127f, 23.0007f
    path.close();
    canvas->drawPath(path, p);
}
}  // END FIDDLE
