// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4590fbf052659d6e629fbfd827081ae5
REG_FIDDLE(Bitmap_readPixels_3, 256, 128, false, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> srcPixels;
    srcPixels.resize(source.height() * source.width() * 8);
    for (int i = 0;  i < 2; ++i) {
    SkPixmap pixmap(SkImageInfo::Make(source.width() * 2, source.height(),
                    i ? kRGBA_8888_SkColorType : kBGRA_8888_SkColorType, kPremul_SkAlphaType),
                    &srcPixels.front() + i * source.width(), source.rowBytes() * 2);
        source.readPixels(pixmap);
    }
    canvas->scale(.25f, .25f);
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(source.width() * 2, source.height()),
                         &srcPixels.front(), source.rowBytes() * 2);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
