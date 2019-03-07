// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=484d60dab5d846bf28c7a4d48892324a
REG_FIDDLE(Surface_022, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorRED);
    SkPaint paint;
    surfCanvas->drawOval({4, 8, 58, 54}, paint);
    SkImageInfo info = SkImageInfo::Make(64, 64, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkData> data(SkData::MakeUninitialized(info.minRowBytes() * info.height()));
    sk_bzero(data->writable_data(), info.minRowBytes() * info.height());
    for (int x : { 32, -32 } ) {
        for (int y : { 32, -32 } ) {
            surf->readPixels(info, data->writable_data(), info.minRowBytes(), x, y);
        }
    }
    sk_sp<SkImage> image = SkImage::MakeRasterData(info, data, info.minRowBytes());
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
