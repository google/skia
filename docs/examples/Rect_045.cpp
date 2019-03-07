// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=85be528a78945a6dc4f7dccb80a80746
REG_FIDDLE(Rect_045, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 30, 50, 40, 60 };
    SkPoint tests[] = { { 30, 50 }, { 39, 49 }, { 29, 59 } };
    for (auto contained : tests) {
        SkDebugf("rect: (%g, %g, %g, %g) %s (%g, %g)\n",
                 rect.left(), rect.top(), rect.right(), rect.bottom(),
                 rect.contains(contained.x(), contained.y()) ? "contains" : "does not contain",
                 contained.x(), contained.y());
    }
}
}  // END FIDDLE
