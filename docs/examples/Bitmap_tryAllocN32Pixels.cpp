// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a2b1e0910f37066f15ae56368775a6d8
REG_FIDDLE(Bitmap_tryAllocN32Pixels, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    if (bitmap.tryAllocN32Pixels(80, 80)) {
        bitmap.eraseColor(SK_ColorTRANSPARENT);
        bitmap.erase(0x7f3f7fff, SkIRect::MakeWH(50, 30));
        bitmap.erase(0x3f7fff3f, SkIRect::MakeXYWH(20, 10, 50, 30));
        bitmap.erase(0x5fff3f7f, SkIRect::MakeXYWH(40, 20, 50, 30));
        canvas->drawImage(bitmap.asImage(), 0, 0);
        for (int x : { 0, 30, 60, 90 } ) {
            canvas->drawImage(bitmap.asImage(), x, 70);
        }
    }
}
}  // END FIDDLE
