// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0de693f4d8dd898a60be8cfba23952be
REG_FIDDLE(Surface_draw, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurface::MakeRasterN32Premul(64, 64));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));
    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorBLACK);
    lil->draw(big->getCanvas(), 16, 16, nullptr);
    SkPixmap pixmap;
    if (big->peekPixels(&pixmap)) {
        SkBitmap bigBits;
        bigBits.installPixels(pixmap);
        canvas->drawBitmap(bigBits, 0, 0);
    }
}
}  // END FIDDLE
