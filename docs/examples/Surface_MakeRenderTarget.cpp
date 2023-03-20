// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=67b6609471a3f1ed0f4b1657004cdecb
REG_FIDDLE(Surface_MakeRenderTarget, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkFont font(nullptr, 32);
    SkPaint paint;

    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
        return;
    }

    SkImageInfo info = SkImageInfo::MakeN32(256, 64, kOpaque_SkAlphaType);
    for (auto surfaceOrigin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin } ) {
        auto gpuSurface(SkSurface::MakeRenderTarget(
                dContext, skgpu::Budgeted::kNo, info, 0, surfaceOrigin, nullptr));
        auto surfaceCanvas = gpuSurface->getCanvas();
        surfaceCanvas->clear(SK_ColorWHITE);
        surfaceCanvas->drawString("GPU rocks!", 20, 40, font, paint);
        sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
        canvas->drawImage(image, 0, 0);
        canvas->translate(0, 128);
    }
}
}  // END FIDDLE
