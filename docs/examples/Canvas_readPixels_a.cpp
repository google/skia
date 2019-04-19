// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=102d014d7f753db2a9b9ee08893aaf11
REG_FIDDLE(Canvas_readPixels_a, 64, 64, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorBLUE);
    SkPaint paint;
    canvas->drawCircle(32, 32, 28, paint);
    SkImageInfo info = SkImageInfo::Make(64, 64, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkData> data(SkData::MakeUninitialized(info.minRowBytes() * info.height()));
    sk_bzero(data->writable_data(), info.minRowBytes() * info.height());
    for (int x : { 32, -32 } ) {
        for (int y : { 32, -32 } ) {
            canvas->readPixels(info, data->writable_data(), info.minRowBytes(), x, y);
        }
    }
    sk_sp<SkImage> image = SkImage::MakeRasterData(info, data, info.minRowBytes());
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
