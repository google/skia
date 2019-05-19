// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ee0185db622602b4eb19583c2f42c734
REG_FIDDLE(IRect_contains_3, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 30, 50, 40, 60 };
    SkIRect tests[] = { { 30, 50, 31, 51}, { 39, 49, 40, 50}, { 29, 59, 30, 60} };
    for (auto contained : tests) {
        SkDebugf("rect: (%d, %d, %d, %d) %s (%d, %d, %d, %d)\n",
                 rect.left(), rect.top(), rect.right(), rect.bottom(),
                 rect.contains(contained) ? "contains" : "does not contain",
                 contained.left(), contained.top(), contained.right(), contained.bottom());
    }
}
}  // END FIDDLE
