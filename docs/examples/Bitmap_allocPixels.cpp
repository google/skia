// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=555c0f62f96602a9dcd459badcd005e0
REG_FIDDLE(Bitmap_allocPixels, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::Make(256, 64, kGray_8_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(info, info.width() * info.bytesPerPixel() + 64);
    SkCanvas offscreen(bitmap);
    offscreen.scale(.5f, .5f);
    for (int y : { 0, 64, 128, 192 } ) {
        offscreen.drawImage(source.asImage(), 0, -y);
        canvas->drawImage(bitmap.asImage(), 0, y);
    }
}
}  // END FIDDLE
