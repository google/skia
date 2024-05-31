// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_AdoptTextureFrom, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());
    // Example does not support DDL.
    if (!dContext) {
        return;
    }
    canvas->scale(.5f, .5f);
    canvas->clear(0x7f3f5f7f);
    int x = 0, y = 0;
    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
        for (auto alpha : { kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType } ) {
            sk_sp<SkImage> image = SkImages::AdoptTextureFrom(
                    dContext, backEndTexture, origin, kRGBA_8888_SkColorType, alpha);
            canvas->drawImage(image, x, y);
            x += 160;
        }
        x -= 160 * 3;
        y += 256;
    }
}
}  // END FIDDLE
