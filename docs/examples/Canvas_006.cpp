//// Copyright 2019 Google LLC.
//// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//#include "fiddle/examples.h"
//REG_FIDDLE(Canvas_006, 256, 256, false, 0) {
//// HASH=b7bc91ff16c9b9351b2a127f35394b82
//void draw(SkCanvas* canvas) {
//    SkBitmap bitmap;
//    bitmap.allocPixels(SkImageInfo::MakeN32Premul(200, 200));
//    {
//        SkCanvas offscreen(bitmap);
//        SkPaint paint;
//        paint.setTextSize(100);
//        offscreen.drawString("ABC", 20, 160, paint);
//        SkRect layerBounds = SkRect::MakeXYWH(32, 32, 192, 192);
//        offscreen.saveLayerAlpha(&layerBounds, 128);
//        offscreen.clear(SK_ColorWHITE);
//        offscreen.drawString("DEF", 20, 160, paint);
//    }
//    canvas->drawBitmap(bitmap, 0, 0, nullptr);
//}
//
//}
