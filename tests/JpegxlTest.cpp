/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_CODEC_DECODES_JPEGXL)
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkJpegxlDecoder.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstdint>

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

DEF_TEST(Jpegxl_Decode1010102, r) {
    // 64x64 10-bit sRGB JXL (solid green: R=117, G=251, B=76 in 8-bit sRGB scale).
    static constexpr uint8_t kRgb10BitSrgbJxl[] = {
            0xff, 0x0a, 0x4f, 0x50, 0x24, 0x08, 0x04, 0x01, 0x00, 0x44, 0x00, 0x4b, 0x18, 0x8b,
            0x15, 0x80, 0xdd, 0xfe, 0xff, 0x00, 0xe2, 0x3f, 0xf7, 0x05, 0x80, 0x21, 0x9d, 0xff,
    };
    auto data = SkData::MakeWithoutCopy(kRgb10BitSrgbJxl, sizeof(kRgb10BitSrgbJxl));

    auto androidCodec = SkAndroidCodec::MakeFromData(data);
    REPORTER_ASSERT(r, androidCodec);
    if (!androidCodec) {
        return;
    }
    REPORTER_ASSERT(
            r, androidCodec->computeOutputColorType(kN32_SkColorType) == kRGBA_1010102_SkColorType);
    REPORTER_ASSERT(r,
                    androidCodec->computeOutputColorType(kRGBA_1010102_SkColorType) ==
                            kRGBA_1010102_SkColorType);

    SkImageInfo decodeInfo = androidCodec->getInfo()
                                     .makeColorType(kRGBA_1010102_SkColorType)
                                     .makeAlphaType(kOpaque_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(decodeInfo);
    SkCodec::Result result = androidCodec->getAndroidPixels(decodeInfo, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

DEF_TEST(Jpegxl_AlphaPremulAndUnpremul, r) {
    // 64x64 8-bit sRGB RGBA JXL with unpremultiplied pixel (R=117, G=251, B=76, A=127).
    static constexpr uint8_t kRgba8BitSrgbUnpremulJxl[] = {
            0xff, 0x0a, 0x4f, 0xc0, 0x4a, 0x08, 0x10, 0x10, 0x00, 0x8c, 0x00, 0x4b,
            0x18, 0x8b, 0x15, 0x00, 0xd4, 0x88, 0x31, 0xc6, 0x8d, 0xbb, 0x7b, 0x00,
            0x07, 0x80, 0x44, 0x01, 0xc0, 0x67, 0x4d, 0xfd, 0xff, 0x6f, 0x13, 0x17,
            0x14, 0xdd, 0xdc, 0xc1, 0xca, 0x1e, 0xbf, 0x27, 0xf9, 0x01,
    };
    // 64x64 8-bit sRGB RGBA JXL with associated (premultiplied) pixel (R=58, G=125, B=38, A=127).
    static constexpr uint8_t kRgba8BitSrgbPremulJxl[] = {
            0xff, 0x0a, 0x4f, 0xc0, 0x00, 0x28, 0x01, 0x08, 0x10, 0x10, 0x00, 0x84,
            0x00, 0x4b, 0x18, 0x8b, 0x15, 0x00, 0x94, 0x88, 0x31, 0x6e, 0xdc, 0xdd,
            0x03, 0x38, 0x80, 0x53, 0x0a, 0x00, 0xc5, 0xd5, 0x74, 0x5b, 0x85, 0x06,
            0xff, 0xff, 0xdc, 0xda, 0xa1, 0x50, 0xd2, 0xf0, 0x65, 0x7e,
    };

    for (sk_sp<SkData> data : {
                 SkData::MakeWithoutCopy(kRgba8BitSrgbUnpremulJxl, sizeof(kRgba8BitSrgbUnpremulJxl)),
                 SkData::MakeWithoutCopy(kRgba8BitSrgbPremulJxl, sizeof(kRgba8BitSrgbPremulJxl)),
         }) {
        SkCodec::Result result;
        auto codec = SkJpegxlDecoder::Decode(data, &result);
        REPORTER_ASSERT(r, codec && result == SkCodec::kSuccess);
        if (!codec) {
            continue;
        }

        // Requesting kOpaque_SkAlphaType on a non-opaque image must fail with kInvalidConversion.
        {
            SkImageInfo opaqueInfo = codec->getInfo()
                                             .makeColorType(kRGBA_8888_SkColorType)
                                             .makeAlphaType(kOpaque_SkAlphaType);
            SkBitmap bm;
            bm.allocPixels(opaqueInfo);
            REPORTER_ASSERT(r,
                            codec->getPixels(opaqueInfo, bm.getPixels(), bm.rowBytes()) ==
                                    SkCodec::kInvalidConversion);
        }

        // RGBA_8888 + kPremul_SkAlphaType (no colorXform): expect premultiplied (58, 125, 38, 127).
        {
            SkImageInfo info = codec->getInfo()
                                       .makeColorType(kRGBA_8888_SkColorType)
                                       .makeAlphaType(kPremul_SkAlphaType);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r,
                            codec->getPixels(info, bm.getPixels(), bm.rowBytes()) ==
                                    SkCodec::kSuccess);
            const uint8_t* px = static_cast<const uint8_t*>(bm.getAddr(0, 0));
            REPORTER_ASSERT(r, px[0] == 58 && px[1] == 125 && px[2] == 38 && px[3] == 127);
        }

        // RGBA_8888 + kUnpremul_SkAlphaType (no colorXform): expect unpremultiplied (117, 251, 76, 127).
        {
            SkImageInfo info = codec->getInfo()
                                       .makeColorType(kRGBA_8888_SkColorType)
                                       .makeAlphaType(kUnpremul_SkAlphaType);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r,
                            codec->getPixels(info, bm.getPixels(), bm.rowBytes()) ==
                                    SkCodec::kSuccess);
            const uint8_t* px = static_cast<const uint8_t*>(bm.getAddr(0, 0));
            REPORTER_ASSERT(r, px[0] == 117 && px[1] == 251 && px[2] == 76 && px[3] == 127);
        }

        // BGRA_8888 + kPremul_SkAlphaType (no colorXform): expect premultiplied BGRA (38, 125, 58, 127).
        {
            SkImageInfo info = codec->getInfo()
                                       .makeColorType(kBGRA_8888_SkColorType)
                                       .makeAlphaType(kPremul_SkAlphaType);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r,
                            codec->getPixels(info, bm.getPixels(), bm.rowBytes()) ==
                                    SkCodec::kSuccess);
            const uint8_t* px = static_cast<const uint8_t*>(bm.getAddr(0, 0));
            REPORTER_ASSERT(r, px[0] == 38 && px[1] == 125 && px[2] == 58 && px[3] == 127);
        }

        // BGRA_8888 + kUnpremul_SkAlphaType (no colorXform): expect unpremultiplied BGRA (76, 251, 117, 127).
        {
            SkImageInfo info = codec->getInfo()
                                       .makeColorType(kBGRA_8888_SkColorType)
                                       .makeAlphaType(kUnpremul_SkAlphaType);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(r,
                            codec->getPixels(info, bm.getPixels(), bm.rowBytes()) ==
                                    SkCodec::kSuccess);
            const uint8_t* px = static_cast<const uint8_t*>(bm.getAddr(0, 0));
            REPORTER_ASSERT(r, px[0] == 76 && px[1] == 251 && px[2] == 117 && px[3] == 127);
        }
    }
}

#endif  // defined(SK_CODEC_DECODES_JPEGXL)
