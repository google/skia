#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=72c35baaeddca1d912edf93d19429c8e
REG_FIDDLE(ImageInfo_height, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->translate(10, 20);
    canvas->drawBitmap(source, 0, 0);
    SkImageInfo imageInfo = source.info();
    SkPaint paint;
    canvas->drawLine(imageInfo.width() + 10, 0, imageInfo.width() + 10, imageInfo.height(), paint);
    canvas->drawString("height", imageInfo.width() + 15, imageInfo.height() / 2, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
