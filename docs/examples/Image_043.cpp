//// Copyright 2019 Google LLC.
//// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//#include "fiddle/examples.h"
//REG_FIDDLE(Image_043, 256, 256, false, 5) {
//// HASH=dbf5f75c1275a3013672f896767140fb
//void draw(SkCanvas* canvas) {
//    sk_sp<SkColorSpace> normalColorSpace = SkColorSpace::MakeRGB(
//             SkColorSpace::kSRGB_RenderTargetGamma, SkColorSpace::kSRGB_Gamut);
//    sk_sp<SkColorSpace> wackyColorSpace = normalColorSpace->makeColorSpin();
//    for (auto colorSpace : { normalColorSpace, wackyColorSpace  } ) {
//        sk_sp<SkImage> colorSpaced = image->makeColorSpace(colorSpace);
//        canvas->drawImage(colorSpaced, 0, 0);
//        canvas->translate(128, 0);
//    }
//}
//}
