// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b034517e39394b7543f06ec885e36d7d
REG_FIDDLE(Image_MakeFromAdoptedTexture, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    if (!canvas->getGrContext()) {
        return;
    }
    canvas->scale(.5f, .5f);
    canvas->clear(0x7f3f5f7f);
    int x = 0, y = 0;
    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
        for (auto alpha : { kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType } ) {
            sk_sp<SkImage> image = SkImage::MakeFromAdoptedTexture(canvas->getGrContext(),
                                                                   backEndTexture, origin,
                                                                   kRGBA_8888_SkColorType, alpha);
            canvas->drawImage(image, x, y);
            x += 160;
        }
        x -= 160 * 3;
        y += 256;
    }
}
}  // END FIDDLE
