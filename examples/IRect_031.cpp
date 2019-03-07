// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(IRect_031, 256, 256, true, 0);
// HASH=3fc62ca29428195f33a3a02b3eb74e4f
void draw(SkCanvas* canvas) {
    SkIRect rect = { 10, 14, 50, 73 };
    rect.outset(5, 13);
    SkDebugf("rect: %d, %d, %d, %d\n", rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
}
}
