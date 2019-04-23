// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c79a196278c58b34cd5f551b0124ecc9
REG_FIDDLE(Bitmap_height, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeA8(16, 32);
    SkBitmap bitmap;
    bitmap.setInfo(info);
    SkDebugf("bitmap height: %d  info height: %d\n", bitmap.height(), info.height());
}
}  // END FIDDLE
