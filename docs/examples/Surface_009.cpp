#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5c7629c15e9ac93f098335e72560fa2e
REG_FIDDLE(Surface_009, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(32);
    GrContext* context = canvas->getGrContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, paint);
         return;
    }
    SkImageInfo info = SkImageInfo::MakeN32(256, 64, kOpaque_SkAlphaType);
    auto gpuSurface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    auto surfaceCanvas = gpuSurface->getCanvas();
    surfaceCanvas->clear(SK_ColorWHITE);
    surfaceCanvas->drawString("GPU rocks!", 20, 40, paint);
    sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
