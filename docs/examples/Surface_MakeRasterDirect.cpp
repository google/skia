// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3f5aeb870104187643197354a7f1d27a
REG_FIDDLE(Surface_MakeRasterDirect, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    const size_t size = info.computeMinByteSize();
    SkAutoTMalloc<SkPMColor> storage(size);
    SkPMColor* pixels = storage.get();
    sk_sp<SkSurface> surface(SkSurface::MakeRasterDirect(info, pixels, info.minRowBytes()));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPMColor pmWhite = pixels[0];
    SkPaint paint;
    canvas->drawPoint(1, 1, paint);
    canvas->flush();  // ensure that point was drawn
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            SkDebugf("%c", *pixels++ == pmWhite ? '-' : 'x');
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
