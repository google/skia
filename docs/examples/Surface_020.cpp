#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8c6184f22cfe068f021704cf92a147a1
REG_FIDDLE(Surface_peekPixels, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorRED);
    SkPaint paint;
    paint.setTextSize(40);
    surfCanvas->drawString("&", 16, 48, paint);
    SkPixmap pixmap;
    if (surf->peekPixels(&pixmap)) {
        SkBitmap surfBits;
        surfBits.installPixels(pixmap);
        canvas->drawBitmap(surfBits, 0, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
