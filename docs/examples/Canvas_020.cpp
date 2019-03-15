// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=af6dec8ef974aa67bf102f29915bcd6a
REG_FIDDLE(Canvas_020, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(0x8055aaff);
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(1, 1));
    canvas->readPixels(bitmap, 0, 0);
    SkDebugf("pixel = %08x\n", bitmap.getAddr32(0, 0)[0]);
}
}  // END FIDDLE
