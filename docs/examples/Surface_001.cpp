// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8e6530b26ab4096a9a91cfaadda1c568
REG_FIDDLE(Surface_001, 256, 256, true, 0) {
static void release_direct_surface_storage(void* pixels, void* context) {
    if (pixels == context) {
        SkDebugf("expected release context\n");
    }
    sk_free(pixels);
}

void draw(SkCanvas* ) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    const size_t rowBytes = info.minRowBytes();
    void* pixels = sk_malloc_throw(info.computeByteSize(rowBytes));
    sk_sp<SkSurface> surface(SkSurface::MakeRasterDirectReleaseProc(info, pixels, rowBytes,
            release_direct_surface_storage, pixels));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPMColor* colorPtr = (SkPMColor*) pixels;
    SkPMColor pmWhite = colorPtr[0];
    SkPaint paint;
    canvas->drawPoint(1, 1, paint);
    canvas->flush();  // ensure that point was drawn
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            SkDebugf("%c", *colorPtr++ == pmWhite ? '-' : 'x');
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
