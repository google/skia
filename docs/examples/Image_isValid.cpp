// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=afc62f38aebc56af8e425297ec67dd37
REG_FIDDLE(Image_isValid, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
        return;
    }

    auto drawImage = [=](sk_sp<SkImage> image, const char* label) -> void {
        if (nullptr == image) {
            return;
        }
        SkFont font;
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas->drawImage(image, 0, 0);
        canvas->drawString(label, image->width() / 2, image->height() / 4, font, paint);
        if (dContext) {
            const char* msg = image->isValid(dContext) ? "is valid on GPU"
                                                       : "not valid on GPU";
            canvas->drawString(msg, 20, image->height() * 5 / 8, font, paint);
        }

        const char* msg = image->isValid(nullptr) ? "is valid on CPU" : "not valid on CPU";

        canvas->drawString(msg, 20, image->height() * 7 / 8, font, paint);
    };
    sk_sp<SkImage> bitmapImage(source.asImage());
    sk_sp<SkImage> textureImage(SkImage::MakeFromTexture(dContext, backEndTexture,
                                                         kTopLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType,
                                                         kOpaque_SkAlphaType, nullptr));
    drawImage(image, "image");
    canvas->translate(image->width(), 0);
    drawImage(bitmapImage, "source");
    canvas->translate(-image->width(), image->height());
    drawImage(textureImage, "backEndTexture");
}
}  // END FIDDLE
