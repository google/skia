#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=900c0eab8dfdecd8301ed5be95887f8e
REG_FIDDLE(Image_peekPixels, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(12, 11));
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorWHITE);
    SkPaint paint;
    offscreen.drawString("%", 1, 10, paint);
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    SkPixmap pixmap;
    if (image->peekPixels(&pixmap)) {
        const SkPMColor* pixels = pixmap.addr32();
        SkPMColor pmWhite = pixels[0];
        for (int y = 0; y < image->height(); ++y) {
            for (int x = 0; x < image->width(); ++x) {
                SkDebugf("%c", *pixels++ == pmWhite ? '-' : 'x');
            }
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
