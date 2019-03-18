// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=acbd79ffb304f332e4b38ef18e19663e
REG_FIDDLE(IRect_contains_4, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 30, 50, 40, 60 };
    SkRect tests[] = { { 30, 50, 31, 51}, { 39, 49, 40, 50}, { 29, 59, 30, 60} };
    for (auto contained : tests) {
        SkDebugf("rect: (%d, %d, %d, %d) %s (%g, %g, %g, %g)\n",
                 rect.left(), rect.top(), rect.right(), rect.bottom(),
                 rect.contains(contained) ? "contains" : "does not contain",
                 contained.left(), contained.top(), contained.right(), contained.bottom());
    }
}
}  // END FIDDLE
