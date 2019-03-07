// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f98cc0451c6e77a8833d261c9a484c5f
REG_FIDDLE(Bitmap_056, 256, 140, false, 5) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(source.width() - 5, source.height() - 5,
                   kGray_8_SkColorType, kOpaque_SkAlphaType), source.rowBytes());
    bitmap.setPixelRef(sk_ref_sp(source.pixelRef()), 5, 5);
    canvas->drawBitmap(bitmap, 10, 10);
}
}  // END FIDDLE
