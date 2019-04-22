// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=53c212c4f2449df0b0eedbc6227b6ab7
REG_FIDDLE(ImageInfo_equal1_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info1 = SkImageInfo::Make(10, 20, kGray_8_SkColorType, kPremul_SkAlphaType);
    SkImageInfo info2 = SkImageInfo::Make(20, 10, kAlpha_8_SkColorType, kUnpremul_SkAlphaType);
    SkDebugf("info1 %c= info2\n", info1 == info2 ? '=' : '!');
    info2 = info2.makeWH(10, 20);
    SkDebugf("info1 %c= info2\n", info1 == info2 ? '=' : '!');
    info2 = info2.makeColorType(kGray_8_SkColorType);
    SkDebugf("info1 %c= info2\n", info1 == info2 ? '=' : '!');
    info2 = info2.makeAlphaType(kPremul_SkAlphaType);
    SkDebugf("info1 %c= info2\n", info1 == info2 ? '=' : '!');
}
}  // END FIDDLE
