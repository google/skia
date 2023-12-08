// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c26cfae4c42cb445240335cc12a50235
REG_FIDDLE(Canvas_const_SkBitmap_const_SkSurfaceProps, 256, 256, true, 0) {
void draw(SkCanvas* ) {
    SkBitmap bitmap;
    // create a bitmap 5 wide and 11 high
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(5, 11));
    SkCanvas canvas(bitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    canvas.clear(SK_ColorWHITE);  // white is Unpremultiplied, in ARGB order
    SkPixmap pixmap;  // provides guaranteed access to the drawn pixels
    if (!canvas.peekPixels(&pixmap)) {
        SkDebugf("peekPixels should never fail.\n");
    }
    const SkPMColor* pixels = pixmap.addr32();  // points to top-left of bitmap
    SkPMColor pmWhite = pixels[0];  // the Premultiplied format may vary
    SkPaint paint;  // by default, draws black, 12 point text
    SkFont font = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));
    canvas.drawString("!", 1, 10, font, paint);  // 1 char at baseline (1, 10)
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            SkDebugf("%c", *pixels++ == pmWhite ? '-' : 'x');
        }
        SkDebugf("\n");
    }
}
}  // END FIDDLE
