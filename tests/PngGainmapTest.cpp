/*
 * Copyright 2025 Google Inc.
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
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstring>
#include <memory>
#include <utility>

namespace {

// Return true if the relative difference between x and y is less than epsilon.
static bool approx_eq(float x, float y, float epsilon) {
    float numerator = std::abs(x - y);
    // To avoid being too sensitive around zero, set the minimum denominator to epsilon.
    float denominator = std::max(std::min(std::abs(x), std::abs(y)), epsilon);
    if (numerator / denominator > epsilon) {
        return false;
    }
    return true;
}

static bool approx_eq(const SkColor4f& x, const SkColor4f& y, float epsilon) {
    return approx_eq(x.fR, y.fR, epsilon) && approx_eq(x.fG, y.fG, epsilon) &&
           approx_eq(x.fB, y.fB, epsilon);
}

template <typename Reporter>
void expect_approx_eq_info(Reporter& r, const SkGainmapInfo& a, const SkGainmapInfo& b) {
    float kEpsilon = 1e-4f;
    REPORTER_ASSERT(r, approx_eq(a.fGainmapRatioMin, b.fGainmapRatioMin, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fGainmapRatioMin, b.fGainmapRatioMin, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fGainmapGamma, b.fGainmapGamma, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fEpsilonSdr, b.fEpsilonSdr, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fEpsilonHdr, b.fEpsilonHdr, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fDisplayRatioSdr, b.fDisplayRatioSdr, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fDisplayRatioHdr, b.fDisplayRatioHdr, kEpsilon));
    REPORTER_ASSERT(r, a.fType == b.fType);
    REPORTER_ASSERT(r, a.fBaseImageType == b.fBaseImageType);

    REPORTER_ASSERT(r, !!a.fGainmapMathColorSpace == !!b.fGainmapMathColorSpace);
    if (a.fGainmapMathColorSpace) {
        skcms_TransferFunction a_fn;
        skcms_Matrix3x3 a_m;
        a.fGainmapMathColorSpace->transferFn(&a_fn);
        a.fGainmapMathColorSpace->toXYZD50(&a_m);
        skcms_TransferFunction b_fn;
        skcms_Matrix3x3 b_m;
        b.fGainmapMathColorSpace->transferFn(&b_fn);
        b.fGainmapMathColorSpace->toXYZD50(&b_m);

        REPORTER_ASSERT(r, approx_eq(a_fn.g, b_fn.g, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.a, b_fn.a, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.b, b_fn.b, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.c, b_fn.c, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.d, b_fn.d, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.e, b_fn.e, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.f, b_fn.f, kEpsilon));

        // The round-trip of the color space through the ICC profile loses significant precision.
        // Use a larger epsilon for it.
        const float kMatrixEpsilon = 1e-2f;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                REPORTER_ASSERT(r, approx_eq(a_m.vals[i][j], b_m.vals[i][j], kMatrixEpsilon));
            }
        }
    }
}

}  // namespace

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
        expect_approx_eq_info(r, rec.info, gainmapInfo);
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

        expect_approx_eq_info(r, sourceGainmapInfo, gainmapInfo);
    }
}
