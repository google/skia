// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8f463ed17b0ed4fb9c503a0ec71706f9
REG_FIDDLE(Bitmap_notifyPixelsChanged, 256, 20, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorRED);
    canvas->scale(16, 16);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    *(SkPMColor*) bitmap.getPixels() = SkPreMultiplyColor(SK_ColorBLUE);
    canvas->drawImage(bitmap.asImage(), 2, 0);
    bitmap.notifyPixelsChanged();
    *(SkPMColor*) bitmap.getPixels() = SkPreMultiplyColor(SK_ColorGREEN);
    canvas->drawImage(bitmap.asImage(), 4, 0);
}
}  // END FIDDLE
