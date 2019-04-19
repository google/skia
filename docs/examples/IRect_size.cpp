// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8b3224641cb3053a7b8a5798b6cd1cf6
REG_FIDDLE(IRect_size, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkIRect& rect) -> void {
        SkISize size = rect.size();
        SkDebugf("%s ", prefix);
        SkDebugf("rect: %d, %d, %d, %d  ", rect.left(), rect.top(), rect.right(), rect.bottom());
        SkDebugf("size: %d, %d\n", size.width(), size.height());
    };
    SkIRect rect = {20, 30, 40, 50};
    debugster("original", rect);
    rect.offset(20, 20);
    debugster("  offset", rect);
    rect.outset(20, 20);
    debugster("  outset", rect);
}
}  // END FIDDLE
