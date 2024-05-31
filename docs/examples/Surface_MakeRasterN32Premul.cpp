// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Surface_MakeRasterN32Premul, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(3, 3)));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPixmap pixmap;
    if (surface->peekPixels(&pixmap)) {
        const uint32_t* colorPtr = pixmap.addr32();
        SkPMColor pmWhite = colorPtr[0];
        SkPaint paint;
        canvas->drawPoint(1, 1, paint);
        for (int y = 0; y < surface->height(); ++y) {
            for (int x = 0; x < surface->width(); ++x) {
                SkDebugf("%c", colorPtr[x] == pmWhite ? '-' : 'x');
            }
            colorPtr += surface->width();
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
