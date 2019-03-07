//// Copyright 2019 Google LLC.
//// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//#include "fiddle/examples.h"
//REG_FIDDLE(Canvas_105, 256, 200, false, 0) {
//// HASH=55f5e59350622c5e2834d1c85789f732
//void draw(SkCanvas* canvas) {
//    SkPaint paint;
//    paint.setAntiAlias(true);
//    float textSizes[] = { 12, 18, 24, 36 };
//    for (auto size: textSizes ) {
//        paint.setTextSize(size);
//        canvas->drawText("Aa", 2, 10, 20, paint);
//        canvas->translate(0, size * 2);
//    }
//    paint.reset();
//    paint.setAntiAlias(true);
//    float yPos = 20;
//    for (auto size: textSizes ) {
//        float scale = size / 12.f;
//        canvas->resetMatrix();
//        canvas->translate(100, 0);
//        canvas->scale(scale, scale);
//        canvas->drawText("Aa", 2, 10 / scale, yPos / scale, paint);
//        yPos += size * 2;
//    }
//}
//
//}
