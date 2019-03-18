// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=286072f8c27ff15be9eb945fa38dc9f7
REG_FIDDLE(Rect_notequal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect test = {0, 0, 2, SK_ScalarNaN};
    SkDebugf("test with NaN is %s" "equal to itself\n", test == test ? "" : "not ");
}
}  // END FIDDLE
