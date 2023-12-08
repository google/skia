// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5c7629c15e9ac93f098335e72560fa2e
REG_FIDDLE(Surface_MakeRenderTarget_3, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 32);
    SkPaint paint;
    auto context = canvas->recordingContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, font, paint);
         return;
    }
    SkImageInfo info = SkImageInfo::MakeN32(256, 64, kOpaque_SkAlphaType);
    auto gpuSurface(SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, info));
    auto surfaceCanvas = gpuSurface->getCanvas();
    surfaceCanvas->clear(SK_ColorWHITE);
    surfaceCanvas->drawString("GPU rocks!", 20, 40, font, paint);
    sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
