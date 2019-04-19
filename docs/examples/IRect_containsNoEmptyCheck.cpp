// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fef2a36bee224e92500199fa9d3cbb8b
REG_FIDDLE(IRect_containsNoEmptyCheck, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 30, 50, 40, 60 };
    SkIRect tests[] = { { 30, 50, 31, 51}, { 39, 49, 40, 50}, { 29, 59, 30, 60} };
    for (auto contained : tests) {
        bool success = rect.containsNoEmptyCheck(
                 contained.left(), contained.top(), contained.right(), contained.bottom());
        SkDebugf("rect: (%d, %d, %d, %d) %s (%d, %d, %d, %d)\n",
                 rect.left(), rect.top(), rect.right(), rect.bottom(),
                 success ? "contains" : "does not contain",
                 contained.left(), contained.top(), contained.right(), contained.bottom());
    }
}
}  // END FIDDLE
