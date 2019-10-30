#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b8216a9e5ff5bc61a0e46eba7d36307b
REG_FIDDLE(Alpha_Type_Unpremul, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor color = SkColorSetARGB(150, 50, 100, 255);
    SkString s;
    s.printf("%u %u %u %u", SkColorGetA(color), SkColorGetR(color),
                            SkColorGetG(color), SkColorGetB(color));
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawString(s, 10, 62, paint);
    canvas->scale(50, 50);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(1, 1, kN32_SkColorType, kUnpremul_SkAlphaType);
    if (bitmap.installPixels(imageInfo, (void*) &color, imageInfo.minRowBytes())) {
        canvas->drawBitmap(bitmap, 0, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
