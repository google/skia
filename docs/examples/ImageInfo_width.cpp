#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e2491817695290d0218be77f091b8460
REG_FIDDLE(ImageInfo_width, 256, 96, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->translate(10, 10);
    canvas->drawBitmap(source, 0, 0);
    SkImageInfo imageInfo = source.info();
    canvas->translate(0, imageInfo.height());
    SkPaint paint;
    canvas->drawLine(0, 10, imageInfo.width(), 10, paint);
    canvas->drawString("width", imageInfo.width() / 2 - 15, 25, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
