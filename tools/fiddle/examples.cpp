// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "examples.h"

template sk_tools::Registry<fiddle::Example>* sk_tools::Registry<fiddle::Example>::gHead;
GrBackendTexture backEndTexture;
GrBackendRenderTarget backEndRenderTarget;
GrBackendTexture backEndTextureRenderTarget;
SkBitmap source;
sk_sp<SkImage> image;
double duration;
double frame;

static sk_sp<SkImage> make_img(int i) {
    auto path = SkStringPrintf("resources/images/example_%d.png", i);
    return SkImage::MakeFromEncoded(SkData::MakeFromFileName(path.c_str()));
}
static SkBitmap to_bmp(const sk_sp<SkImage>& image) {
    SkBitmap bmp;
    SkAssertResult(image && image->asLegacyBitmap(&bmp));
    return bmp;
}

int main() {
    sk_sp<SkImage> images[6] = {
        make_img(1),
        make_img(2),
        make_img(3),
        make_img(4),
        make_img(5),
        make_img(6),
    };
    SkBitmap bitmaps[6] = {
        to_bmp(images[0]),
        to_bmp(images[1]),
        to_bmp(images[2]),
        to_bmp(images[3]),
        to_bmp(images[4]),
        to_bmp(images[5]),
    };
    std::unique_ptr<SkCanvas> canvas = SkMakeNullCanvas();
    for (const fiddle::Example& example : sk_tools::Registry<fiddle::Example>::Range()) {
         image = images[example.fImageIndex];
         source = bitmaps[example.fImageIndex];
         SkDebugf("==> %s\n", example.fName);
         example.fFunc(canvas.get());
    }
}
