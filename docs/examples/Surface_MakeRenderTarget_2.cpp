#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=640321e8ecfb3f9329f3bc6e1f02485f
REG_FIDDLE(Surface_MakeRenderTarget_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    auto test_draw = [](SkCanvas* surfaceCanvas) -> void {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setColor(0xFFBBBBBB);
        surfaceCanvas->drawRect(SkRect::MakeWH(128, 64), paint);
        paint.setColor(SK_ColorWHITE);
        paint.setTextSize(32);
        surfaceCanvas->drawString("Pest", 0, 25, paint);
    };
    GrContext* context = canvas->getGrContext();
    SkImageInfo info = SkImageInfo::MakeN32(128, 64, kOpaque_SkAlphaType);
    int y = 0;
    for (auto geometry : { kRGB_H_SkPixelGeometry, kBGR_H_SkPixelGeometry,
                           kRGB_V_SkPixelGeometry, kBGR_V_SkPixelGeometry } ) {
        SkSurfaceProps props(0, geometry);
        sk_sp<SkSurface> surface = context ? SkSurface::MakeRenderTarget(
                context, SkBudgeted::kNo, info, 0, &props) : SkSurface::MakeRaster(info, &props);
        test_draw(surface->getCanvas());
        surface->draw(canvas, 0, y, nullptr);
        sk_sp<SkImage> image(surface->makeImageSnapshot());
        SkAutoCanvasRestore acr(canvas, true);
        canvas->scale(8, 8);
        canvas->drawImage(image, 12, y / 8);
        y += 64;
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
