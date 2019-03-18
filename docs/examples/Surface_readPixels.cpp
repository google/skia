#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9f454fb93bca6482598d198b4121f0a6
REG_FIDDLE(Surface_readPixels, 256, 32, false, 0) {
void draw(SkCanvas* canvas) {
    sk_sp<SkSurface> surf(SkSurface::MakeRasterN32Premul(64, 64));
    auto surfCanvas = surf->getCanvas();
    surfCanvas->clear(SK_ColorRED);
    SkPaint paint;
    paint.setTextSize(40);
    surfCanvas->drawString("&", 0, 32, paint);
    std::vector<SkPMColor> storage;
    storage.resize(surf->width() * surf->height());
    SkPixmap pixmap(SkImageInfo::MakeN32Premul(32, 32), &storage.front(),
                    surf->width() * sizeof(storage[0]));
    if (surf->readPixels(pixmap, 0, 0)) {
        SkBitmap surfBits;
        surfBits.installPixels(pixmap);
        canvas->drawBitmap(surfBits, 0, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
