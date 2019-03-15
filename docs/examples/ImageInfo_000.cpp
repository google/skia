#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=79146a1a41d58d22582fdc567c6ffe4e
REG_FIDDLE(ImageInfo_000, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPMColor color = SkPreMultiplyARGB(255, 50, 100, 150);
    SkString s;
    s.printf("%u %u %u %u", SkColorGetA(color), SkColorGetR(color),
                            SkColorGetG(color), SkColorGetB(color));
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString(s, 10, 62, paint);
    canvas->scale(50, 50);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(1, 1, kN32_SkColorType, kOpaque_SkAlphaType);
    if (bitmap.installPixels(imageInfo, (void*) &color, imageInfo.minRowBytes())) {
        canvas->drawBitmap(bitmap, 0, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
