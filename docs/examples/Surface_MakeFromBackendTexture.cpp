#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d3aec071998f871809f515e58abb1b0e
REG_FIDDLE(Surface_MakeFromBackendTexture, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(32);
    GrContext* context = canvas->getGrContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, paint);
         return;
    }
    sk_sp<SkSurface> gpuSurface = SkSurface::MakeFromBackendTexture(context,
            backEndTexture, kTopLeft_GrSurfaceOrigin, 0,
            kRGBA_8888_SkColorType, nullptr, nullptr);
    auto surfaceCanvas = gpuSurface->getCanvas();
    surfaceCanvas->drawString("GPU rocks!", 20, 40, paint);
    sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
