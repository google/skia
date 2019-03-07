// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c6197d204ef9e4ccfb583242651fb2a7
REG_FIDDLE(Surface_003, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPixmap pixmap;
    if (surface->peekPixels(&pixmap)) {
        const uint32_t* colorPtr = pixmap.addr32();
        SkPMColor pmWhite = colorPtr[0];
        SkPaint paint;
        canvas->drawPoint(1, 1, paint);
        canvas->flush();  // ensure that point was drawn
        for (int y = 0; y < info.height(); ++y) {
            for (int x = 0; x < info.width(); ++x) {
                SkDebugf("%c", colorPtr[x] == pmWhite ? '-' : 'x');
            }
            colorPtr += info.width();
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
