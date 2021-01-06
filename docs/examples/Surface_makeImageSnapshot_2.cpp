// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b18b8ab693b09eb70a1d22ab63790cc7
REG_FIDDLE(Surface_makeImageSnapshot_2, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> big(SkSurface::MakeRasterN32Premul(64, 64));
    sk_sp<SkSurface> lil(big->makeSurface(SkImageInfo::MakeN32(32, 32, kPremul_SkAlphaType)));
    big->getCanvas()->clear(SK_ColorRED);
    lil->getCanvas()->clear(SK_ColorBLACK);
    sk_sp<SkImage> early(big->makeImageSnapshot());
    lil->draw(big->getCanvas(), 16, 16);
    sk_sp<SkImage> later(big->makeImageSnapshot({0, 0, 16, 16}));
    canvas->drawImage(early, 0, 0);
    canvas->drawImage(later, 0, 0);
}
}  // END FIDDLE
