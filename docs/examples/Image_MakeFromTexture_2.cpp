// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=2b1e46354d823dbb53fa6af570135329
REG_FIDDLE(Image_MakeFromTexture_2, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
<<<<<<< HEAD
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
=======
    auto* rContext = canvas->recordingContext();
    if (!rContext) {
>>>>>>> 64f7eae510... git squash commit for refactoring.
       return;
    }
    GrDirectContext *dContext = rContext->asDirectContext();
    if (!dContext) {
        return;
    }

    auto releaseCallback = [](SkImage::ReleaseContext releaseContext) -> void {
       *((int *) releaseContext) += 128;
    };
    int x = 0, y = 0;
    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
        sk_sp<SkImage> image = SkImage::MakeFromTexture(dContext, backEndTexture,
<<<<<<< HEAD
               origin, kRGBA_8888_SkColorType, kOpaque_SkAlphaType, nullptr, debugster, &x);
=======
                                                        origin, kRGBA_8888_SkColorType,
                                                        kOpaque_SkAlphaType, nullptr,
                                                        releaseCallback, &x);
>>>>>>> 64f7eae510... git squash commit for refactoring.
        canvas->drawImage(image, x, y);
        y += 128;
    }
}
}  // END FIDDLE
