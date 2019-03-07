#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=eeec9e07e604b44d0208899a2fe5bef5
REG_FIDDLE(Image_036, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    auto drawImage = [=](sk_sp<SkImage> image, GrContext* context, const char* label) -> void {
        if (nullptr == image || nullptr == context) {
            return;
        }
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_sp<SkImage> texture(image->makeTextureImage(context, nullptr));
        canvas->drawImage(texture, 0, 0);
        canvas->drawString(label, 20, texture->height() / 4, paint);
    };
    sk_sp<SkImage> bitmapImage(SkImage::MakeFromBitmap(source));
    GrContext* context = canvas->getGrContext();
    sk_sp<SkImage> textureImage(SkImage::MakeFromTexture(context, backEndTexture,
                                kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                kOpaque_SkAlphaType, nullptr));
    drawImage(image, context, "image");
    canvas->translate(image->width(), 0);
    drawImage(bitmapImage, context, "source");
    canvas->translate(-image->width(), image->height());
    drawImage(textureImage, context, "backEndTexture");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
