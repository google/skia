// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=eeec9e07e604b44d0208899a2fe5bef5
REG_FIDDLE(Image_TextureFromImage, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
        return;
    }

    auto drawImage = [=](sk_sp<SkImage> image,
                         GrDirectContext* dContext,
                         const char* label) -> void {
        if (nullptr == image || nullptr == dContext) {
            return;
        }

        SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 12);

        SkPaint paint;
        paint.setAntiAlias(true);

        sk_sp<SkImage> texture = SkImages::TextureFromImage(dContext, image);
        canvas->drawImage(texture, 0, 0);
        canvas->drawString(label, 20, texture->height() / 4, font, paint);
    };
    sk_sp<SkImage> bitmapImage(source.asImage());

    sk_sp<SkImage> textureImage(SkImages::BorrowTextureFrom(dContext,
                                                            backEndTexture,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            kRGBA_8888_SkColorType,
                                                            kOpaque_SkAlphaType,
                                                            nullptr));
    drawImage(image, dContext, "image");
    canvas->translate(image->width(), 0);
    drawImage(bitmapImage, dContext, "source");
    canvas->translate(-image->width(), image->height());
    drawImage(textureImage, dContext, "backEndTexture");
}
}  // END FIDDLE
