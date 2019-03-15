// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d8439ba8d23a424fa032fb97147fd2d2
REG_FIDDLE(Rect_019, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect tests[] = {{20, 30, 41, 51}, {-20, -30, -41, -51}};
    for (auto rect : tests) {
        SkDebugf("left: %3g right: %3g centerX: %3g\n", rect.left(), rect.right(), rect.centerX());
        rect.sort();
        SkDebugf("left: %3g right: %3g centerX: %3g\n", rect.left(), rect.right(), rect.centerX());
    }
}
}  // END FIDDLE
