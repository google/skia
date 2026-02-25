// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Surface_MakeRasterDirect, 256, 256, true, 0) {
void draw(SkCanvas*) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    const size_t size = info.computeMinByteSize();
    SkPMColor* pixels = new SkPMColor[size];
    SkPMColor* raw_pixels = pixels; // Save the original address to prevent memory free crashes
    sk_sp<SkSurface> surface(SkSurfaces::WrapPixels(info, pixels, info.minRowBytes()));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPMColor pmWhite = pixels[0];
    SkPaint paint;
    canvas->drawPoint(1, 1, paint);
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            SkDebugf("%c", *pixels++ == pmWhite ? '-' : 'x');
        }
        SkDebugf("\n");
    }
    delete[] raw_pixels;
}
}  // END FIDDLE
