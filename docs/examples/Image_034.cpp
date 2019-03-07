#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=80856fe921ce36f8d5a32d8672bccbfc
REG_FIDDLE(Image_034, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    struct {
        const char* name;
        sk_sp<SkImage> image;
    } tests[] = { { "image", image }, { "bitmap", SkImage::MakeFromBitmap(source) },
          { "texture", SkImage::MakeFromTexture(canvas->getGrContext(), backEndTexture,
                            kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                            kOpaque_SkAlphaType, nullptr) } };
    SkString string;
    SkPaint paint;
    for (const auto& test : tests ) {
        if (!test.image) {
            string.printf("no %s", test.name);
        } else {
            string.printf("%s" "encoded %s", test.image->refEncodedData() ? "" : "no ", test.name);
        }
        canvas->drawString(string, 10, 20, paint);
        canvas->translate(0, 20);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
