// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c6586ff8d24869c780169b0d19c75df6
REG_FIDDLE(IRect_002, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkSize size = {25.5f, 35.5f};
    SkIRect rect = SkIRect::MakeSize(size.toRound());
    SkDebugf("round width: %d  height: %d\n", rect.width(), rect.height());
    rect = SkIRect::MakeSize(size.toFloor());
    SkDebugf("floor width: %d  height: %d\n", rect.width(), rect.height());
}
}  // END FIDDLE
