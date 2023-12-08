// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d093aad721261f421c4bef4a296aab48
REG_FIDDLE(Image_GetBackendTextureFromImage, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkFont font = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));
    SkPaint paint;

    GrRecordingContext* context = canvas->recordingContext();
    if (!context) {
        canvas->drawString("GPU only!", 20, 40, font, paint);
        return;
    }
    GrDirectContext* direct = context->asDirectContext();
    if (!direct) {
        canvas->drawString("Direct context only!", 20, 40, font, paint);
        return;
    }

    sk_sp<SkImage> imageFromBackend = SkImages::AdoptTextureFrom(direct,
                                                                 backEndTexture,
                                                                 kBottomLeft_GrSurfaceOrigin,
                                                                 kRGBA_8888_SkColorType,
                                                                 kOpaque_SkAlphaType);
    GrBackendTexture textureFromImage;
    if (!SkImages::GetBackendTextureFromImage(imageFromBackend, &textureFromImage, false)) {
        return;
    }

    sk_sp<SkImage> imageFromTexture = SkImages::AdoptTextureFrom(direct,
                                                                 textureFromImage,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 kRGBA_8888_SkColorType,
                                                                 kOpaque_SkAlphaType);
    canvas->drawImage(imageFromTexture, 0, 0);
    canvas->drawImage(imageFromBackend, 128, 128);
}
}  // END FIDDLE
