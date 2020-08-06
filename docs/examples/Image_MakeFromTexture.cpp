// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=94e9296c53bad074bf2a48ff885dac13
REG_FIDDLE(Image_MakeFromTexture, 256, 128, false, 3) {
void draw(SkCanvas* canvas) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
       return;
    }
    canvas->scale(.25f, .25f);
    int x = 0;
    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
        sk_sp<SkImage> image = SkImage::MakeFromTexture(context, backEndTexture,
               origin, kRGBA_8888_SkColorType, kOpaque_SkAlphaType, nullptr);
        canvas->drawImage(image, x, 0);
    x += 512;
    }
}
}  // END FIDDLE
