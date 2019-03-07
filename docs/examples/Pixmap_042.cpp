// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e18549b5ee1039cb61b0bb38c2104fc9
REG_FIDDLE(Pixmap_042, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
    std::vector<int32_t> srcPixels;
    const int rowBytes = image->width() * 4;
    srcPixels.resize(image->height() * rowBytes);
    SkPixmap pixmap(info, (const void*) &srcPixels.front(), rowBytes);
    image->readPixels(pixmap, 0, 0);
    for (int index = 0; index < 3; ++index ) {
        std::vector<int32_t> dstPixels;
        dstPixels.resize(image->height() * rowBytes);
        SkPixmap dstmap(info, &dstPixels.front(), rowBytes);
        pixmap.readPixels(dstmap);
        SkBitmap bitmap;
        bitmap.installPixels(dstmap);
        canvas->translate(32, 32);
        canvas->drawBitmap(bitmap, 0, 0);
    }
}
}  // END FIDDLE
