// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Surface_draw, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64)));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));
    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorBLACK);
    lil->draw(big->getCanvas(), 16, 16);
    SkPixmap pixmap;
    if (big->peekPixels(&pixmap)) {
        SkBitmap bigBits;
        bigBits.installPixels(pixmap);
        canvas->drawImage(bigBits.asImage(), 0, 0);
    }
}
}  // END FIDDLE
