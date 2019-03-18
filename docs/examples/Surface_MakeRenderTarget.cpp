#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=67b6609471a3f1ed0f4b1657004cdecb
REG_FIDDLE(Surface_MakeRenderTarget, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(32);
    GrContext* context = canvas->getGrContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, paint);
         return;
    }
    SkImageInfo info = SkImageInfo::MakeN32(256, 64, kOpaque_SkAlphaType);
    for (auto surfaceOrigin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin } ) {
        auto gpuSurface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, 0,
               surfaceOrigin, nullptr));
        auto surfaceCanvas = gpuSurface->getCanvas();
        surfaceCanvas->clear(SK_ColorWHITE);
        surfaceCanvas->drawString("GPU rocks!", 20, 40, paint);
        sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
        canvas->drawImage(image, 0, 0);
       canvas->translate(0, 128);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
