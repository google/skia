// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ebeeafafeb8fe39d5ffc9115b02c2340
REG_FIDDLE(Rect_centerY, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 2e+38f, 2e+38f, 3e+38f, 3e+38f };
    SkDebugf("left: %g right: %g centerX: %g ", rect.left(), rect.right(), rect.centerX());
    SkDebugf("safe mid x: %g\n", rect.left() / 2 + rect.right() / 2);
}
}  // END FIDDLE
