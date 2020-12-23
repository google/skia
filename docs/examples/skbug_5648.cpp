// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skbug_5648, 256, 256, true, 4) {
// skbug_5648
void draw(SkCanvas*) {
    SkBitmap bitmap;
    source.extractSubset(&bitmap, {0, 0, source.width() - 1, source.height() - 1});
    auto img0 = bitmap.asImage();
    auto img1 = bitmap.asImage();
    SkDebugf("%u\n%u\n", img0->uniqueID(), img1->uniqueID());
}
}  // END FIDDLE
