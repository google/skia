// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9def507d2295f7051effd0c83bb04436
REG_FIDDLE(ImageInfo_047, 256, 130, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(2, 2);
    const size_t size = info.computeByteSize(100000);
    SkAutoTMalloc<SkPMColor> storage(size);
    SkPMColor* pixels = storage.get();
    SkBitmap bitmap;
    bitmap.setInfo(info);
    bitmap.setPixels(pixels);
    bitmap.eraseColor(SK_ColorRED);
    canvas->scale(50, 50);
    canvas->rotate(8);
    canvas->drawBitmap(bitmap, 2, 0);
}
}  // END FIDDLE
