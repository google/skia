/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "tests/Test.h"

#include <memory>
#include <utility>

DEF_TEST(Codec_recommendedF16, r) {
    // Encode an F16 bitmap. SkPngEncoder will encode this to a true-color PNG
    // with a bit depth of 16. SkAndroidCodec should always recommend F16 for
    // such a PNG.
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(10, 10, kRGBA_F16_SkColorType,
            kPremul_SkAlphaType, SkColorSpace::MakeSRGB()));
    // What is drawn is not important.
    bm.eraseColor(SK_ColorBLUE);

    SkDynamicMemoryWStream wstream;
    REPORTER_ASSERT(r, SkPngEncoder::Encode(&wstream, bm.pixmap(), {}));
    auto data = wstream.detachAsData();
    auto androidCodec = SkAndroidCodec::MakeFromData(std::move(data));
    if (!androidCodec) {
        ERRORF(r, "Failed to create SkAndroidCodec");
        return;
    }

    REPORTER_ASSERT(r, androidCodec->computeOutputColorType(kN32_SkColorType)
            == kRGBA_F16_SkColorType);
}
