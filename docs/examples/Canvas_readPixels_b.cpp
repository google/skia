// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=481e990e923a0ed34654f4361b94f096
REG_FIDDLE(Canvas_readPixels_b, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(0x8055aaff);
    for (SkAlphaType alphaType : { kPremul_SkAlphaType, kUnpremul_SkAlphaType } ) {
        uint32_t pixel = 0;
        SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, alphaType);
        if (canvas->readPixels(info, &pixel, 4, 0, 0)) {
            SkDebugf("pixel = %08x\n", pixel);
        }
    }
}
}  // END FIDDLE
