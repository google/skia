#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=afc62f38aebc56af8e425297ec67dd37
REG_FIDDLE(Image_isValid, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    auto drawImage = [=](sk_sp<SkImage> image, const char* label) -> void {
        if (nullptr == image) {
            return;
        }
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas->drawImage(image, 0, 0);
        canvas->drawString(label, image->width() / 2, image->height() / 4, paint);
        if (canvas->getGrContext()) {
            canvas->drawString(image->isValid(canvas->getGrContext()) ? "is valid on GPU" :
                    "not valid on GPU", 20, image->height() * 5 / 8, paint);
        }
        canvas->drawString(image->isValid(nullptr) ? "is valid on CPU" :
                "not valid on CPU", 20, image->height() * 7 / 8, paint);
    };
    sk_sp<SkImage> bitmapImage(SkImage::MakeFromBitmap(source));
    sk_sp<SkImage> textureImage(SkImage::MakeFromTexture(canvas->getGrContext(), backEndTexture,
                                kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                kOpaque_SkAlphaType, nullptr));
    drawImage(image, "image");
    canvas->translate(image->width(), 0);
    drawImage(bitmapImage, "source");
    canvas->translate(-image->width(), image->height());
    drawImage(textureImage, "backEndTexture");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
