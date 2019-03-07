//// Copyright 2019 Google LLC.
//// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//#include "fiddle/examples.h"
//REG_FIDDLE(Bitmap_007, 256, 256, true, 0) {
//// HASH=7f972d742dd78d2500034d8867e9ef2f
//void draw(SkCanvas* canvas) {
//    SkBitmap bitmap;
//    bitmap.allocPixels(SkImageInfo::MakeN32Premul(10, 11));
//    SkCanvas offscreen(bitmap);
//    offscreen.clear(SK_ColorWHITE);
//    SkPaint paint;
//    offscreen.drawString("&", 0, 10, paint);
//    const SkPixmap& pixmap = bitmap.pixmap();
//    if (pixmap.addr()) {
//        SkPMColor pmWhite = *pixmap.addr32(0, 0);
//        for (int y = 0; y < pixmap.height(); ++y) {
//            for (int x = 0; x < pixmap.width(); ++x) {
//                SkDebugf("%c", *pixmap.addr32(x, y) == pmWhite ? '-' : 'x');
//            }
//            SkDebugf("\n");
//        }
//    }
//}
//}
