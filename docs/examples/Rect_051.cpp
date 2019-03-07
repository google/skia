// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7145dc17ebce4f54e892102f6c98e811
REG_FIDDLE(Rect_051, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 10, 40, 50, 80 };
    SkDebugf("%s intersection", rect.intersects(30, 60, 70, 90) ? "" : "no ");
}
}  // END FIDDLE
