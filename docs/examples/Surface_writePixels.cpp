#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=760793bcf0ef193fa61ea03e6e8fc825
REG_FIDDLE(Surface_writePixels, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorRED);
    SkPaint paint;
    paint.setTextSize(40);
    surfCanvas->drawString("&", 16, 40, paint);
    SkPixmap pixmap;
    if (surf->peekPixels(&pixmap)) {
        surf->writePixels(pixmap, 25, 25);
        sk_sp<SkImage> image(surf->makeImageSnapshot());
        canvas->drawImage(image, 0, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
