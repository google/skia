// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(UnpremulGPU, 256, 256, false, 6) {
void draw(SkCanvas* canvas) {
    canvas->clear(0xffa020a0);
    SkImageInfo premulInfo = SkImageInfo::MakeS32(10, 10, kPremul_SkAlphaType);
    SkBitmap premulBitmap;
    premulBitmap.allocPixels(premulInfo);
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            *(premulBitmap.getAddr32(x, y)) = 0x80808080;
        }
    }
    sk_sp<SkImage> premulImage = premulBitmap.asImage();

    SkImageInfo unpremulInfo = premulInfo.makeAlphaType(kUnpremul_SkAlphaType);
    SkBitmap unpremulBitmap;
    unpremulBitmap.allocPixels(unpremulInfo);
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            *(unpremulBitmap.getAddr32(x, y)) = 0x80FFFFFF;
        }
    }
    sk_sp<SkImage> unpremulImage = unpremulBitmap.asImage();

    SkPaint paint;
    const SkTileMode tile = SkTileMode::kRepeat;
    paint.setShader(premulImage->makeShader(tile, tile, SkSamplingOptions()));
    canvas->drawCircle(10.0f, 10.0f, 10.0f, paint);

    paint.setShader(unpremulImage->makeShader(tile, tile, SkSamplingOptions()));
    canvas->drawCircle(10.0f, 35.0f, 10.0f, paint);
}
}  // END FIDDLE
