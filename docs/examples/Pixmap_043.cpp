// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8e3c8a9c1d0d2e9b8bf66e24d274f792
REG_FIDDLE(Pixmap_043, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
    std::vector<int32_t> srcPixels;
    int rowBytes = image->width() * 4;
    srcPixels.resize(image->height() * rowBytes);
    SkPixmap pixmap(info, (const void*) &srcPixels.front(), rowBytes);
    image->readPixels(pixmap, 0, 0);
    for (int offset : { 32, 64, 96 } ) {
        info = SkImageInfo::MakeN32Premul(image->width() + offset, image->height());
        rowBytes = info.width() * 4;
        std::vector<int32_t> dstPixels;
        dstPixels.resize(image->height() * rowBytes);
        SkPixmap dstmap(info, &dstPixels.front(), rowBytes);
        pixmap.scalePixels(dstmap, kMedium_SkFilterQuality);
        SkBitmap bitmap;
        bitmap.installPixels(dstmap);
        canvas->translate(32, 32);
        canvas->drawBitmap(bitmap, 0, 0);
    }
}
}  // END FIDDLE
