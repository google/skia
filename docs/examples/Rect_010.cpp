// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=443fe5f8296d4cdb19cc9862a9cf77a4
REG_FIDDLE(Rect_isFinite, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect largest = { SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax };
        SkDebugf("largest is finite: %s\n", largest.isFinite() ? "true" : "false");
        SkDebugf("large width %g\n", largest.width());
        SkRect widest = SkRect::MakeWH(largest.width(), largest.height());
        SkDebugf("widest is finite: %s\n", widest.isFinite() ? "true" : "false");
}
}  // END FIDDLE
