// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=22e7ce79ab2fe94252d23319f2258127
REG_FIDDLE(Image_MakeRasterData, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    size_t rowBytes = image->width() * SkColorTypeBytesPerPixel(kRGBA_8888_SkColorType);
    sk_sp<SkData> data = SkData::MakeUninitialized(rowBytes * image->height());
    SkImageInfo dstInfo = SkImageInfo::MakeN32(image->width(), image->height(),
                                               kPremul_SkAlphaType);
    image->readPixels(dstInfo, data->writable_data(), rowBytes, 0, 0, SkImage::kAllow_CachingHint);
    sk_sp<SkImage> raw = SkImage::MakeRasterData(dstInfo.makeColorType(kRGBA_8888_SkColorType),
                                                 data, rowBytes);
    canvas->drawImage(image, 0, 0);
    canvas->drawImage(raw.get(), 128, 0);
}
}  // END FIDDLE
