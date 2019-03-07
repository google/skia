//// Copyright 2019 Google LLC.
//// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//#include "fiddle/examples.h"
//REG_FIDDLE(ImageInfo_016, 256, 32, false, 0) {
//// HASH=f206f698e7a8db3d84334c26b1a702dc
//void draw(SkCanvas* canvas) {
//    SkImageInfo imageInfo;
//    size_t rowBytes;
//    SkIPoint origin;
//    (void) canvas->accessTopLayerPixels(&imageInfo, &rowBytes, &origin);
//    const char* alphaType[] = { "Unknown", "Opaque", "Premul", "Unpremul" };
//    SkString string;
//    string.printf("k%s_SkAlphaType", alphaType[(int) imageInfo.alphaType()]);
//    SkPaint paint;
//    canvas->drawString(string, 20, 20, paint);
//}
//}
