#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=33d0c5ad5a4810e533ae1010e29f8b75
REG_FIDDLE(Surface_getCanvas, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(64, 64));
    SkCanvas* surfaceCanvas = surface->getCanvas();
    surfaceCanvas->clear(SK_ColorBLUE);
    SkPaint paint;
    paint.setTextSize(40);
    surfaceCanvas->drawString("\xF0\x9F\x98\x81", 12, 45, paint);
    surface->draw(canvas, 0, 0, nullptr);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
