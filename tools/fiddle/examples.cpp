// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "examples.h"

template sk_tools::Registry<fiddle::Example>* sk_tools::Registry<fiddle::Example>::gHead;

// These globals are needed by fiddles:
GrBackendTexture backEndTexture;
GrBackendRenderTarget backEndRenderTarget;
GrBackendTexture backEndTextureRenderTarget;
SkBitmap source;
sk_sp<SkImage> image;
double duration = 1.0;
double frame = 1.0;

int main() {
    sk_sp<SkImage> images[7];
    SkBitmap bitmaps[7];
    for (int i = 1; i < 7; ++i) {
        SkString path = SkStringPrintf("resources/images/example_%d.png", i);
        images[i] = SkImage::MakeFromEncoded(SkData::MakeFromFileName(path.c_str()));
        SkAssertResult(images[i] && images[i]->asLegacyBitmap(&bitmaps[i]));
    }
    for (const fiddle::Example& example : sk_tools::Registry<fiddle::Example>::Range()) {
        SkASSERT(example.fImageIndex < 7);
        image = images[example.fImageIndex];
        source = bitmaps[example.fImageIndex];
        SkBitmap bmp;
        bmp.allocN32Pixels(example.fWidth, example.fHeight);
        bmp.eraseColor(SK_ColorWHITE);
        SkCanvas canvas(bmp);
        SkDebugf("==> %s\n", example.fName);
        example.fFunc(&canvas);
    }
}
