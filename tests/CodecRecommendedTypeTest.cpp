/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodec.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkEncodedImageFormat.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkStream.h"

#include "Test.h"

DEF_TEST(Codec_recommendedF16, r) {
    // Encode an F16 bitmap. SkEncodeImage will encode this to a true-color PNG
    // with a bit depth of 16. SkAndroidCodec should always recommend F16 for
    // such a PNG.
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(10, 10, kRGBA_F16_SkColorType,
            kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear()));
    // What is drawn is not important.
    bm.eraseColor(SK_ColorBLUE);

    SkDynamicMemoryWStream wstream;
    REPORTER_ASSERT(r, SkEncodeImage(&wstream, bm, SkEncodedImageFormat::kPNG, 100));
    auto data = wstream.detachAsData();
    auto androidCodec = SkAndroidCodec::MakeFromData(std::move(data));
    if (!androidCodec) {
        ERRORF(r, "Failed to create SkAndroidCodec");
        return;
    }

    REPORTER_ASSERT(r, androidCodec->computeOutputColorType(kN32_SkColorType)
            == kRGBA_F16_SkColorType);
}
