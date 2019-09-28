#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d093aad721261f421c4bef4a296aab48
REG_FIDDLE(Image_getBackendTexture, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    GrContext* grContext = canvas->getGrContext();
    if (!grContext) {
        canvas->drawString("GPU only!", 20, 40, SkPaint());
        return;
    }
    sk_sp<SkImage> imageFromBackend = SkImage::MakeFromAdoptedTexture(grContext, backEndTexture,
            kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
    GrBackendTexture textureFromImage = imageFromBackend->getBackendTexture(false);
    if (!textureFromImage.isValid()) {
        return;
    }
    sk_sp<SkImage> imageFromTexture = SkImage::MakeFromAdoptedTexture(grContext, textureFromImage,
            kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
    canvas->drawImage(imageFromTexture, 0, 0);
    canvas->drawImage(imageFromBackend, 128, 128);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
