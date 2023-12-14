/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkGainmapShader.h"
#include "tests/Test.h"

static bool approx_equal(const SkColor4f& a, const SkColor4f& b) {
    constexpr float kEpsilon = 1e-3f;
    return std::abs(a.fR - b.fR) < kEpsilon && std::abs(a.fG - b.fG) < kEpsilon &&
           std::abs(a.fB - b.fB) < kEpsilon && std::abs(a.fA - b.fA) < kEpsilon;
}

// Create a 1x1 image with a specified color.
static sk_sp<SkImage> make_1x1_image(
        sk_sp<SkColorSpace> imageColorSpace,
        SkAlphaType imageAlphaType,
        SkColor4f imageColor,
        sk_sp<SkColorSpace> imageColorColorSpace = SkColorSpace::MakeSRGBLinear()) {
    SkImageInfo bmInfo =
            SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, imageAlphaType, imageColorSpace);
    SkBitmap bm;
    bm.allocPixels(bmInfo);

    SkImageInfo writePixelsInfo = SkImageInfo::Make(
            1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, imageColorColorSpace);
    SkPixmap writePixelsPixmap(writePixelsInfo, &imageColor, writePixelsInfo.minRowBytes());
    bm.writePixels(writePixelsPixmap, 0, 0);
    return SkImages::RasterFromBitmap(bm);
}

// Return gainmap info that will scale 1 up to the specified hdrRatioMax.
static SkGainmapInfo simple_gainmap_info(float hdrRatioMax) {
    SkGainmapInfo gainmapInfo;
    gainmapInfo.fDisplayRatioSdr = 1.f;
    gainmapInfo.fDisplayRatioHdr = hdrRatioMax;
    gainmapInfo.fEpsilonSdr = {0.f, 0.f, 0.f, 1.f};
    gainmapInfo.fEpsilonHdr = {0.f, 0.f, 0.f, 1.f};
    gainmapInfo.fGainmapRatioMin = {1.f, 1.f, 1.f, 1.f};
    gainmapInfo.fGainmapRatioMax = {hdrRatioMax, hdrRatioMax, hdrRatioMax, 1.f};
    return gainmapInfo;
}

// Draw using a gainmap to a canvas with the specified HDR to SDR ratio and the specified color
// space. Return the result as unpremultiplied sRGB linear.
static SkColor4f draw_1x1_gainmap(sk_sp<SkImage> baseImage,
                                  sk_sp<SkImage> gainmapImage,
                                  const SkGainmapInfo& gainmapInfo,
                                  float dstRatio,
                                  sk_sp<SkColorSpace> dstColorSpace = SkColorSpace::MakeSRGB()) {
    constexpr auto kRect = SkRect::MakeWH(1.f, 1.f);
    SkImageInfo canvasInfo =
            SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType, dstColorSpace);
    SkBitmap canvasBitmap;
    canvasBitmap.allocPixels(canvasInfo);
    canvasBitmap.eraseColor(SK_ColorTRANSPARENT);

    sk_sp<SkShader> shader = SkGainmapShader::Make(baseImage,
                                                   kRect,
                                                   SkSamplingOptions(),
                                                   gainmapImage,
                                                   kRect,
                                                   SkSamplingOptions(),
                                                   gainmapInfo,
                                                   kRect,
                                                   dstRatio,
                                                   dstColorSpace);
    SkPaint paint;
    paint.setShader(shader);
    SkCanvas canvas(canvasBitmap);
    canvas.drawRect(kRect, paint);

    SkColor4f result = {0.f, 0.f, 0.f, 0.f};
    SkImageInfo readPixelsInfo = SkImageInfo::Make(
            1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGBLinear());
    canvas.readPixels(readPixelsInfo, &result, sizeof(result), 0, 0);
    return result;
}

// Verify that the gainmap shader correctly applies the base, gainmap, and destination rectangles.
DEF_TEST(GainmapShader_rects, r) {
    SkColor4f sdrColors[5][2] = {
            {{-1.f, -1.f, -1.f, 1.0f}, {-1.f, -1.f, -1.f, 1.0f}},
            {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.5f, 1.0f}},
            {{1.0f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.5f, 1.0f}},
            {{0.5f, 1.0f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.5f, 1.0f}},
            {{0.5f, 0.5f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}},
    };
    SkPixmap sdrPixmap(SkImageInfo::Make(2, 5, kRGBA_F32_SkColorType, kOpaque_SkAlphaType),
                       sdrColors,
                       2 * sizeof(SkColor4f));
    auto sdrImage = SkImages::RasterFromPixmap(sdrPixmap, nullptr, nullptr);
    const auto sdrImageRect = SkRect::MakeXYWH(0.f, 1.f, 2.f, 4.f);

    // The top pixel indicates to gain only red, and the bottom pixel indicates to gain everything
    // except red.
    SkColor4f gainmapColors[2][2] = {
            {{-1.f, -1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f, 1.f}},
            {{-1.f, -1.f, -1.f, 1.f}, {0.0f, 1.0f, 1.0f, 1.f}},
    };
    SkPixmap gainmapPixmap(SkImageInfo::Make(2, 2, kRGBA_F32_SkColorType, kOpaque_SkAlphaType),
                           gainmapColors,
                           2 * sizeof(SkColor4f));
    auto gainmapImage = SkImages::RasterFromPixmap(gainmapPixmap, nullptr, nullptr);
    const auto gainmapImageRect = SkRect::MakeXYWH(1.f, 0.f, 1.f, 2.f);
    SkGainmapInfo gainmapInfo = simple_gainmap_info(2.f);
    gainmapInfo.fEpsilonHdr[0] = 0.1f;

    SkImageInfo canvasInfo = SkImageInfo::Make(
            4, 6, kRGBA_F32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkBitmap canvasBitmap;
    canvasBitmap.allocPixels(canvasInfo);
    canvasBitmap.eraseColor(SK_ColorTRANSPARENT);
    const auto canvasRect = SkRect::MakeXYWH(1.f, 1.f, 2.f, 4.f);

    sk_sp<SkShader> shader = SkGainmapShader::Make(sdrImage,
                                                   sdrImageRect,
                                                   SkSamplingOptions(),
                                                   gainmapImage,
                                                   gainmapImageRect,
                                                   SkSamplingOptions(),
                                                   gainmapInfo,
                                                   canvasRect,
                                                   gainmapInfo.fDisplayRatioHdr,
                                                   canvasInfo.refColorSpace());
    SkPaint paint;
    paint.setShader(shader);
    SkCanvas canvas(canvasBitmap);
    canvas.drawRect(canvasRect, paint);

    // Compute and compare the expected colors.
    // This is linearToSRGB(srgbToLinear(1.0)*2.0) = linearToSRGB(2.0).
    constexpr float k10G = 1.353256028586302f;
    // This is linearToSRGB(srgbToLinear(0.5)*2.0)
    constexpr float k05G = 0.6858361015012847f;
    // The 'R' component also has a fEpsilonHdr set.
    // This is linearToSRGB(srgbToLinear(1.0)*2.0-0.1) = linearToSRGB(1.9).
    constexpr float kR10G = 1.3234778541409058f;
    // This is linearToSRGB(srgbToLinear(0.5)-0.1)
    // The gain map is 0.f (no gain), but there are still affectd by the offset.
    constexpr float kR05G = 0.371934685412575f;
    SkColor4f expectedColors[4][2] = {
            {{kR10G, 1.0f, 1.0f, 1.0f}, {kR10G, 1.0f, 0.5f, 1.0f}},
            {{kR10G, 0.5f, 1.0f, 1.0f}, {kR10G, 0.5f, 0.5f, 1.0f}},
            {{kR05G, k10G, k10G, 1.0f}, {kR05G, k10G, k05G, 1.0f}},
            {{kR05G, k05G, k10G, 1.0f}, {kR05G, k05G, k05G, 1.0f}},
    };
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 2; ++x) {
            const auto color = canvasBitmap.getColor4f(x + 1, y + 1);
            const auto& expected = expectedColors[y][x];
            REPORTER_ASSERT(r,
                            approx_equal(color, expected),
                            "color (%.3f %.3f %.3f %.3f) does not match expected color (%.3f %.3f "
                            "%.3f %.3f) at "
                            "pixel (%d, %d)",
                            color.fR,
                            color.fG,
                            color.fB,
                            color.fA,
                            expected.fR,
                            expected.fG,
                            expected.fB,
                            expected.fA,
                            x,
                            y);
        }
    }
}

DEF_TEST(GainmapShader_baseImageIsHdr, r) {
    SkColor4f hdrColors[4][2] = {
            {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.5f, 1.0f}},
            {{1.0f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.5f, 1.0f}},
            {{0.5f, 1.0f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.5f, 1.0f}},
            {{0.5f, 0.5f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}},
    };
    SkPixmap hdrPixmap(SkImageInfo::Make(2, 4, kRGBA_F32_SkColorType, kOpaque_SkAlphaType),
                       hdrColors,
                       2 * sizeof(SkColor4f));
    auto hdrImage = SkImages::RasterFromPixmap(hdrPixmap, nullptr, nullptr);
    const auto hdrImageRect = SkRect::MakeXYWH(0.f, 0.f, 2.f, 4.f);

    // The top pixel indicates to gain only red, and the bottom pixel indicates to gain everything
    // except red.
    SkColor4f gainmapColors[2][1] = {
            {{1.0f, 0.0f, 0.0f, 1.f}},
            {{0.0f, 1.0f, 1.0f, 1.f}},
    };
    SkPixmap gainmapPixmap(SkImageInfo::Make(1, 2, kRGBA_F32_SkColorType, kOpaque_SkAlphaType),
                           gainmapColors,
                           1 * sizeof(SkColor4f));
    auto gainmapImage = SkImages::RasterFromPixmap(gainmapPixmap, nullptr, nullptr);
    const auto gainmapImageRect = SkRect::MakeXYWH(0.f, 0.f, 1.f, 2.f);
    SkGainmapInfo gainmapInfo = simple_gainmap_info(2.f);
    gainmapInfo.fBaseImageType = SkGainmapInfo::BaseImageType::kHDR;
    gainmapInfo.fEpsilonSdr[0] = 0.1f;

    SkImageInfo canvasInfo = SkImageInfo::Make(
            2, 4, kRGBA_F32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkBitmap canvasBitmap;
    canvasBitmap.allocPixels(canvasInfo);
    canvasBitmap.eraseColor(SK_ColorTRANSPARENT);
    const auto canvasRect = SkRect::MakeXYWH(0.f, 0.f, 2.f, 4.f);

    sk_sp<SkShader> shader = SkGainmapShader::Make(hdrImage,
                                                   hdrImageRect,
                                                   SkSamplingOptions(),
                                                   gainmapImage,
                                                   gainmapImageRect,
                                                   SkSamplingOptions(),
                                                   gainmapInfo,
                                                   canvasRect,
                                                   gainmapInfo.fDisplayRatioSdr,
                                                   canvasInfo.refColorSpace());
    SkPaint paint;
    paint.setShader(shader);
    SkCanvas canvas(canvasBitmap);
    canvas.drawRect(canvasRect, paint);

    // Compute and compare the expected colors.
    // This is linearToSRGB(srgbToLinear(1.0)*0.5) = linearToSRGB(0.5).
    constexpr float k10G = 0.7353569830524495f;
    // This is linearToSRGB(srgbToLinear(0.5)*0.5)
    constexpr float k05G = 0.3607802138332792f;
    // The 'R' component also has a fEpsilonSdr set.
    // This is linearToSRGB(srgbToLinear(1.0)*0.5-0.1) = linearToSRGB(0.4).
    constexpr float kR10G = 0.6651850846308363f;
    // This is linearToSRGB(srgbToLinear(0.5)-0.1)
    // The gain map is 0.f (no gain), but there are still affectd by the offset.
    constexpr float kR05G = 0.371934685412575f;
    SkColor4f expectedColors[4][2] = {
            {{kR10G, 1.0f, 1.0f, 1.0f}, {kR10G, 1.0f, 0.5f, 1.0f}},
            {{kR10G, 0.5f, 1.0f, 1.0f}, {kR10G, 0.5f, 0.5f, 1.0f}},
            {{kR05G, k10G, k10G, 1.0f}, {kR05G, k10G, k05G, 1.0f}},
            {{kR05G, k05G, k10G, 1.0f}, {kR05G, k05G, k05G, 1.0f}},
    };
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 2; ++x) {
            const auto color = canvasBitmap.getColor4f(x, y);
            const auto& expected = expectedColors[y][x];
            REPORTER_ASSERT(r,
                            approx_equal(color, expected),
                            "color (%.3f %.3f %.3f %.3f) does not match expected color (%.3f %.3f "
                            "%.3f %.3f) at "
                            "pixel (%d, %d)",
                            color.fR,
                            color.fG,
                            color.fB,
                            color.fA,
                            expected.fR,
                            expected.fG,
                            expected.fB,
                            expected.fA,
                            x,
                            y);
        }
    }
}

// Verify that the gainmap shader isn't affected by the color spaces of the base, gainmap, or
// destination. But the fGainmapMathColorSpace is taken into account.
DEF_TEST(GainmapShader_colorSpace, r) {
    auto sdrColorSpace =
            SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kSRGB)->makeColorSpin();
    auto gainmapColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ, SkNamedGamut::kRec2020);
    auto dstColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::kHLG, SkNamedGamut::kDisplayP3);

    constexpr SkColor4f kSdrColor = {0.25f, 0.5f, 1.f, 1.f};
    constexpr SkColor4f kGainmapColor = {
            0.0f,  // The sRGB G channel will have a exp2(0.0)=1.000 gain.
            0.5f,  // The sRGB B channel will have a exp2(0.5)=1.414 gain.
            1.0f,  // The sRGB R channel will have a exp2(1.0)=2.000 gain.
            1.f};
    constexpr SkColor4f kExpectedColor = {0.5f, 0.5f, 1.414f, 1.f};

    auto sdrImage = make_1x1_image(sdrColorSpace, kOpaque_SkAlphaType, kSdrColor);
    auto gainmapImage = make_1x1_image(
            gainmapColorSpace, kOpaque_SkAlphaType, kGainmapColor, gainmapColorSpace);
    SkGainmapInfo gainmapInfo = simple_gainmap_info(2.f);

    auto color = draw_1x1_gainmap(
            sdrImage, gainmapImage, gainmapInfo, gainmapInfo.fDisplayRatioHdr, dstColorSpace);
    REPORTER_ASSERT(r, approx_equal(color, kExpectedColor));

    // Setting fGainmapMathColorSpace to the base image's color space does not change the result.
    gainmapInfo.fGainmapMathColorSpace = sdrColorSpace;
    color = draw_1x1_gainmap(
            sdrImage, gainmapImage, gainmapInfo, gainmapInfo.fDisplayRatioHdr, dstColorSpace);
    REPORTER_ASSERT(r, approx_equal(color, kExpectedColor));

    // Setting fGainmapMathColorSpace ot a different color space does change the result.
    gainmapInfo.fGainmapMathColorSpace =
            SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ, SkNamedGamut::kRec2020);
    color = draw_1x1_gainmap(
            sdrImage, gainmapImage, gainmapInfo, gainmapInfo.fDisplayRatioHdr, dstColorSpace);
    REPORTER_ASSERT(r, !approx_equal(color, kExpectedColor));
}
