// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4f153cf1d0dbe1a95acf5badeec14dae
REG_FIDDLE(Canvas_096, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkIRect center = { 20, 10, 50, 40 };
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(60, 60));
    SkCanvas bitCanvas(bitmap);
    SkPaint paint;
    SkColor gray = 0xFF000000;
    int left = 0;
    for (auto right: { center.fLeft, center.fRight, bitmap.width() } ) {
        int top = 0;
        for (auto bottom: { center.fTop, center.fBottom, bitmap.height() } ) {
            paint.setColor(gray);
            bitCanvas.drawIRect(SkIRect::MakeLTRB(left, top, right, bottom), paint);
            gray += 0x001f1f1f;
            top = bottom;
        }
        left = right;
    }
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    SkImage* imagePtr = image.get();
    for (auto dest: { 20, 30, 40, 60, 90 } ) {
        canvas->drawImageNine(imagePtr, center, SkRect::MakeWH(dest, dest), nullptr);
        canvas->translate(dest + 4, 0);
    }
}
}  // END FIDDLE
