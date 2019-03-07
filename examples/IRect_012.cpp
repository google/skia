// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(IRect_012, 256, 256, true, 0);
// HASH=63977f97999bbd6eecfdcc7575d75492
void draw(SkCanvas* canvas) {
    SkIRect large = { -2147483647, 1, 2147483644, 2 };
    SkDebugf("width: %d width64: %lld\n", large.width(), large.width64());
}
}
