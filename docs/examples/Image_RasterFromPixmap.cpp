// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_RasterFromPixmap, 256, 256, true, 0) {
    static void releaseProc(const void* pixels, SkImages::ReleaseContext context) {
        int* countPtr = static_cast<int*>(context);
        *countPtr += 1;
    }

void draw(SkCanvas* canvas) {
    SkColor color = 0;
    SkPixmap pixmap(SkImageInfo::MakeN32(1, 1, kPremul_SkAlphaType), &color, 4);
    int releaseCount = 0;
    sk_sp<SkImage> image(SkImages::RasterFromPixmap(pixmap, releaseProc, &releaseCount));
    SkDebugf("before reset: %d\n", releaseCount);
    image.reset();
    SkDebugf("after reset: %d\n", releaseCount);
}
}  // END FIDDLE
