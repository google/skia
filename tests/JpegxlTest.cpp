/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_CODEC_DECODES_JPEGXL)
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkJpegxlDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(Jpegxl_DecodeP3, r) {
    const char* path = "images/Webkit-logo-P3.jxl";
    auto data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Failed to find %s", path);
        return;
    }

    REPORTER_ASSERT(r, SkJpegxlDecoder::IsJpegxl(data->data(), data->size()));

    SkCodec::Result result;
    auto codec = SkJpegxlDecoder::Decode(data, &result);
    if (!codec) {
        ERRORF(r, "Could not create codec from %s - error %s", path, SkCodec::ResultToString(result));
        return;
    }
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    REPORTER_ASSERT(r, codec->getEncodedFormat() == SkEncodedImageFormat::kJPEGXL);
    REPORTER_ASSERT(r, codec->dimensions().width() == 1000);
    REPORTER_ASSERT(r, codec->dimensions().height() == 1000);

    // Verify the image resolves to the Display P3 color space.
    SkColorSpace* cs = codec->getInfo().colorSpace();
    REPORTER_ASSERT(r, cs);
    if (cs) {
        sk_sp<SkColorSpace> p3 =
                SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
        REPORTER_ASSERT(r, SkColorSpace::Equals(cs, p3.get()));
    }

    // Verify decoding pixels succeeds.
    SkImageInfo decodeInfo =
            codec->getInfo().makeColorType(kRGBA_8888_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(decodeInfo);
    result = codec->getPixels(decodeInfo, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    // Spot-check the two shades of red in the image:
    // 1. Background red (R=255, G=0, B=0, A=255)
    // 2. WebKit logo red (R=241, G=0, B=0, A=255)
    constexpr SkColor kBgRed = SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00);
    constexpr SkColor kLogoRed = SkColorSetARGB(0xFF, 0xF1, 0x00, 0x00);

    REPORTER_ASSERT(r, bm.getColor(0, 0) == kBgRed);
    REPORTER_ASSERT(r, bm.getColor(100, 100) == kBgRed);
    REPORTER_ASSERT(r, bm.getColor(900, 900) == kBgRed);
    REPORTER_ASSERT(r, bm.getColor(500, 200) == kBgRed);
    REPORTER_ASSERT(r, bm.getColor(500, 800) == kBgRed);

    REPORTER_ASSERT(r, bm.getColor(500, 500) == kLogoRed);
    REPORTER_ASSERT(r, bm.getColor(500, 400) == kLogoRed);
    REPORTER_ASSERT(r, bm.getColor(500, 600) == kLogoRed);
}

#endif  // defined(SK_CODEC_DECODES_JPEGXL)
