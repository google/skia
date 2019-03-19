// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e9bd4f02b6cfb3ac864cb7fee7d7299c
REG_FIDDLE(ImageInfo_isOpaque, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int height = 2;
    const int width = 2;
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
    bitmap.setInfo(imageInfo);
    for (int index = 0; index < 2; ++index) {
        bitmap.allocPixels();
        bitmap.eraseColor(0x00000000);
        SkDebugf("isOpaque: %s\n", imageInfo.isOpaque() ? "true" : "false");
        bitmap.eraseColor(0xFFFFFFFF);
        SkDebugf("isOpaque: %s\n", imageInfo.isOpaque() ? "true" : "false");
        imageInfo = imageInfo.makeAlphaType(kOpaque_SkAlphaType);
        bitmap.setInfo(imageInfo);
    }
}
}  // END FIDDLE
