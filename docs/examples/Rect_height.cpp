// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=39429e45f05240218ecd511443ab3e44
REG_FIDDLE(Rect_height, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 25, 10, 20 };
    SkDebugf("unsorted height: %g\n", unsorted.height());
    SkRect large = { 1, -2147483647.f, 2, 2147483644.f };
    SkDebugf("large height: %.0f\n", large.height());
}
}  // END FIDDLE
