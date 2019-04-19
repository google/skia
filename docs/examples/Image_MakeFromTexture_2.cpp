// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2b1e46354d823dbb53fa6af570135329
REG_FIDDLE(Image_MakeFromTexture_2, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
       return;
    }
    auto debugster = [](SkImage::ReleaseContext releaseContext) -> void {
       *((int *) releaseContext) += 128;
    };
    int x = 0, y = 0;
    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
        sk_sp<SkImage> image = SkImage::MakeFromTexture(context, backEndTexture,
               origin, kRGBA_8888_SkColorType, kOpaque_SkAlphaType, nullptr, debugster, &x);
        canvas->drawImage(image, x, y);
        y += 128;
    }
}
}  // END FIDDLE
