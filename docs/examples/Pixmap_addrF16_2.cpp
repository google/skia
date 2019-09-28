// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f6076cad455bc80af5d06eb121d3b6f2
REG_FIDDLE(Pixmap_addrF16_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    const int wordsPerPixel = 4;
    const int rowWords = w * wordsPerPixel;
    uint16_t storage[rowWords * h];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kRGBA_F16_SkColorType, kPremul_SkAlphaType),
                    storage, rowWords * sizeof(storage[0]));
    SkDebugf("pixmap.addrF16(1, 2) %c= &storage[1 * wordsPerPixel + 2 * rowWords]\n",
              pixmap.addrF16(1, 2)  == &storage[1 * wordsPerPixel + 2 * rowWords] ? '=' : '!');
}
}  // END FIDDLE
