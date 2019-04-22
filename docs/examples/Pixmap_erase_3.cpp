// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=f884f3f46a565f052a5e252ae2f36e9b
REG_FIDDLE(Pixmap_erase_3, 256, 50, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t storage[2];
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 2);
    SkPixmap pixmap(info, storage, info.minRowBytes());
    SkIRect topPixelBounds = {0, 0, 1, 1};
    pixmap.erase({ 0.65f, 0.45f, 0.25f, 1 }, &topPixelBounds);
    SkIRect bottomPixelBounds = {0, 1, 1, 2};
    pixmap.erase({ 0.25f, 0.65f, 0.45f, 1 }, &bottomPixelBounds);
    SkBitmap bitmap;
    canvas->scale(20, 20);
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
