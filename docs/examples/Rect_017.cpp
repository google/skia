// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=11f8f0efe6291019fee0ac17844f6c1a
REG_FIDDLE(Rect_017, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect unsorted = { 15, 25, 10, 5 };
    SkDebugf("unsorted width: %g\n", unsorted.width());
    SkRect large = { -2147483647.f, 1, 2147483644.f, 2 };
    SkDebugf("large width: %.0f\n", large.width());
}
}  // END FIDDLE
