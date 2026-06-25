/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkGainmapShader.h"
#include "tests/GainmapTestCommon.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstring>
#include <memory>
#include <utility>



// Decode an image and its gainmap.
template <typename Reporter>
void decode_all(Reporter& r,
                std::unique_ptr<SkStream> stream,
                SkBitmap& baseBitmap,
                SkBitmap& gainmapBitmap,
                SkGainmapInfo& gainmapInfo,
                bool& decodedGainmap) {
    // Decode the base bitmap.
    SkCodec::Result result = SkCodec::kSuccess;
    std::unique_ptr<SkCodec> baseCodec =
            SkPngDecoder::Decode(std::move(stream), &result);
    REPORTER_ASSERT(r, baseCodec);
    baseBitmap.allocPixels(baseCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == baseCodec->getPixels(baseBitmap.info(),
                                                              baseBitmap.getPixels(),
                                                              baseBitmap.rowBytes()));
    std::unique_ptr<SkAndroidCodec> androidCodec =
            SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
    REPORTER_ASSERT(r, androidCodec);

    // Extract the gainmap info and codec.
    std::unique_ptr<SkAndroidCodec> gainmapCodec;

    decodedGainmap = androidCodec->getGainmapAndroidCodec(&gainmapInfo, &gainmapCodec);
    if (decodedGainmap) {
        REPORTER_ASSERT(r, gainmapCodec);
        // Decode the gainmap bitmap.
        gainmapBitmap.allocPixels(gainmapCodec->getInfo());
        REPORTER_ASSERT(
                r,
                SkCodec::kSuccess == gainmapCodec->getAndroidPixels(gainmapBitmap.info(),
                                                                    gainmapBitmap.getPixels(),
                                                                    gainmapBitmap.rowBytes()));
    }
}

DEF_TEST(AndroidCodec_pngGainmapDecode, r) {
    const struct Rec {
        const char* path;
        SkISize dimensions;
        SkColor originColor;
        SkColor farCornerColor;
        SkGainmapInfo info;
    } recs[] = {
            {"images/gainmap.png",
             SkISize::Make(32, 32),
             0xffffffff,
             0xff000000,
             {{25.f, 0.5f, 1.f, 1.f},
              {2.f, 4.f, 8.f, 1.f},
              {0.5, 1.f, 2.f, 1.f},
              {0.01f, 0.001f, 0.0001f, 1.f},
              {0.0001f, 0.001f, 0.01f, 1.f},
              2.f,
              4.f,
              SkGainmapInfo::BaseImageType::kHDR,
              SkGainmapInfo::Type::kDefault,
              nullptr}},
    };

    for (const auto& rec : recs) {
        auto stream = GetResourceAsStream(rec.path, false);
        REPORTER_ASSERT(r, stream);

        SkBitmap baseBitmap;
        SkBitmap gainmapBitmap;
        SkGainmapInfo gainmapInfo;
        bool decodedGainmap;
        decode_all(r, std::move(stream), baseBitmap, gainmapBitmap, gainmapInfo, decodedGainmap);

        REPORTER_ASSERT(r, decodedGainmap);

        // Spot-check the image size and pixels.
        REPORTER_ASSERT(r, gainmapBitmap.dimensions() == rec.dimensions);
        REPORTER_ASSERT(r, gainmapBitmap.getColor(0, 0) == rec.originColor);
        REPORTER_ASSERT(r,
                        gainmapBitmap.getColor(rec.dimensions.fWidth - 1,
                                               rec.dimensions.fHeight - 1) == rec.farCornerColor);

        // Verify the gainmap rendering parameters.
        skiatest::ExpectApproxEqInfo(r, rec.info, gainmapInfo);
    }
}

DEF_TEST(AndroidCodec_pngGainmapInvalidDecode, r) {
    const char* paths[] = {
            "images/gainmap_no_gdat.png",
            "images/gainmap_gdat_no_gmap.png",

    };

    for (const auto& path : paths) {
        auto stream = GetResourceAsStream(path, false);
        REPORTER_ASSERT(r, stream);

        SkBitmap baseBitmap;
        SkBitmap gainmapBitmap;
        SkGainmapInfo gainmapInfo;
        bool decodedGainmap;
        decode_all(r, std::move(stream), baseBitmap, gainmapBitmap, gainmapInfo, decodedGainmap);

        REPORTER_ASSERT(r, !decodedGainmap);
    }
}

DEF_TEST(AndroidCodec_pngGainmapEncodeAndDecode, r) {
    SkColorType colorTypes[] = {
            kRGBA_8888_SkColorType,
            kAlpha_8_SkColorType,
    };

    for (const auto& colorType : colorTypes) {
        SkDynamicMemoryWStream stream;
        SkGainmapInfo sourceGainmapInfo;
        sourceGainmapInfo.fGainmapRatioMin = {1.f, 1.f, 1.f, 1.f};
        sourceGainmapInfo.fGainmapRatioMax = {5.f, 5.f, 5.f, 1.f};
        sourceGainmapInfo.fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
        sourceGainmapInfo.fEpsilonSdr = {0.01f, 0.01f, 0.01f, 0.01f};
        sourceGainmapInfo.fEpsilonHdr = {0.001f, 0.001f, 0.001f, 0.001f};
        sourceGainmapInfo.fDisplayRatioSdr = 1.f;
        sourceGainmapInfo.fDisplayRatioHdr = 3.f;
        sourceGainmapInfo.fGainmapMathColorSpace = SkColorSpace::MakeSRGB();
        SkBitmap sourceBase;
        sourceBase.allocPixels(
                SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
        sourceBase.eraseColor(SK_ColorRED);
        SkBitmap sourceGainmap;
        sourceGainmap.allocPixels(SkImageInfo::Make(4, 4, colorType, kOpaque_SkAlphaType));
        sourceGainmap.eraseColor(SK_ColorGREEN);

        SkPngEncoder::Options options;
        options.fGainmap = &(sourceGainmap.pixmap());
        options.fGainmapInfo = &sourceGainmapInfo;
        REPORTER_ASSERT(r, SkPngEncoder::Encode(&stream, sourceBase.pixmap(), options));

        SkBitmap baseBitmap;
        SkBitmap gainmapBitmap;
        SkGainmapInfo gainmapInfo;
        bool decodedGainmap;
        decode_all(
                r, stream.detachAsStream(), baseBitmap, gainmapBitmap, gainmapInfo, decodedGainmap);

        REPORTER_ASSERT(r, decodedGainmap);

        REPORTER_ASSERT(r, baseBitmap.dimensions().fHeight == 16);
        REPORTER_ASSERT(r, baseBitmap.dimensions().fWidth == 16);
        REPORTER_ASSERT(r, baseBitmap.getColor(0, 0) == SK_ColorRED);
        REPORTER_ASSERT(r, baseBitmap.getColor(15, 15) == SK_ColorRED);

        REPORTER_ASSERT(r, gainmapBitmap.dimensions().fHeight == 4);
        REPORTER_ASSERT(r, gainmapBitmap.dimensions().fWidth == 4);

        if (colorType == kAlpha_8_SkColorType) {
            REPORTER_ASSERT(r, gainmapBitmap.getColor(0, 0) == SK_ColorBLACK);
            REPORTER_ASSERT(r, gainmapBitmap.getColor(3, 3) == SK_ColorBLACK);
        } else {
            REPORTER_ASSERT(r, gainmapBitmap.getColor(0, 0) == SK_ColorGREEN);
            REPORTER_ASSERT(r, gainmapBitmap.getColor(3, 3) == SK_ColorGREEN);
        }
        // Verify the gainmap rendering parameters.
        if (colorType == kAlpha_8_SkColorType) {
            sourceGainmapInfo.fGainmapMathColorSpace = nullptr;
        }

        skiatest::ExpectApproxEqInfo(r, sourceGainmapInfo, gainmapInfo);
    }
}
