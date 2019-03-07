// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d06880c42f8bb3b4c3b67bd988046049
REG_FIDDLE(Bitmap_009, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeA8(16, 32);
    SkBitmap bitmap;
    bitmap.setInfo(info);
    SkDebugf("bitmap width: %d  info width: %d\n", bitmap.width(), info.width());
}
}  // END FIDDLE
