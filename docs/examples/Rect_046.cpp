// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=92f9e6aa5bb76791139a24cf7d8df99e
REG_FIDDLE(Rect_046, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30, 50, 40, 60 };
    SkRect tests[] = { { 30, 50, 31, 51}, { 39, 49, 40, 50}, { 29, 59, 30, 60} };
    for (auto contained : tests) {
        SkDebugf("rect: (%g, %g, %g, %g) %s (%g, %g, %g, %g)\n",
                 rect.left(), rect.top(), rect.right(), rect.bottom(),
                 rect.contains(contained) ? "contains" : "does not contain",
                 contained.left(), contained.top(), contained.right(), contained.bottom());
    }
}
}  // END FIDDLE
