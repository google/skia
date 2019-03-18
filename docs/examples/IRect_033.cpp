// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a7958a4e0668f5cf805a8e78eb57f51d
REG_FIDDLE(IRect_contains, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 30, 50, 40, 60 };
    SkIPoint pts[] = { { 30, 50}, { 40, 50}, { 30, 60} };
    for (auto pt : pts) {
        SkDebugf("rect: (%d, %d, %d, %d) %s (%d, %d)\n",
                 rect.left(), rect.top(), rect.right(), rect.bottom(),
                 rect.contains(pt.x(), pt.y()) ? "contains" : "does not contain", pt.x(), pt.y());
    }
}
}  // END FIDDLE
