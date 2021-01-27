// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6971489f28291f08e429cc6ccc73b09b
REG_FIDDLE(Color_Constants_c, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    std::vector<uint32_t> srcPixels;
    constexpr int width = 256;
    constexpr int height = 256;
    srcPixels.resize(width * height);
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(width, height);
    SkPixmap pixmap(imageInfo, &srcPixels.front(), imageInfo.minRowBytes());
    pixmap.erase(SK_ColorTRANSPARENT);
    pixmap.erase(SK_ColorRED, { 24, 24, 192, 192 } );
    pixmap.erase(SK_ColorBLACK, { 48, 48, 168, 168 } );
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    canvas->drawImage(bitmap.asImage(), 48, 48);
}
}  // END FIDDLE
