// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=85f199032943b6483722c34a91c4e20f
REG_FIDDLE(Canvas_readPixels_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(0x8055aaff);
    uint32_t pixels[1] = { 0 };
    SkPixmap pixmap(SkImageInfo::MakeN32Premul(1, 1), pixels, 4);
    canvas->readPixels(pixmap, 0, 0);
    SkDebugf("pixel = %08x\n", pixels[0]);
}
}  // END FIDDLE
