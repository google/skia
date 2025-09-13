/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkGainmapShader.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkImageInfoPriv.h"

#include <cmath>
#include <cstdint>

static constexpr char gGainmapSKSL[] =
        "uniform shader base;"
        "uniform shader gainmap;"
        "uniform half4 logRatioMin;"
        "uniform half4 logRatioMax;"
        "uniform half4 gainmapGamma;"
        "uniform half4 epsilonBase;"
        "uniform half4 epsilonOther;"
        "uniform half W;"
        "uniform int gainmapIsAlpha;"
        "uniform int gainmapIsRed;"
        "uniform int singleChannel;"
        "uniform int noGamma;"
        "uniform int isApple;"
        "uniform half appleG;"
        "uniform half appleH;"
        ""
        "half4 main(float2 coord) {"
            "half4 S = base.eval(coord);"
            "half4 G = gainmap.eval(coord);"
            "if (gainmapIsAlpha == 1) {"
                "G = half4(G.a, G.a, G.a, 1.0);"
            "}"
            "if (gainmapIsRed == 1) {"
                "G = half4(G.r, G.r, G.r, 1.0);"
            "}"
            "if (singleChannel == 1) {"
                "half L;"
                "if (isApple == 1) {"
                    "L = pow(G.r, appleG);"
                    "L = log(1.0 + (appleH - 1.0) * pow(G.r, appleG));"
                "} else if (noGamma == 1) {"
                    "L = mix(logRatioMin.r, logRatioMax.r, G.r);"
                "} else {"
                    "L = mix(logRatioMin.r, logRatioMax.r, pow(G.r, gainmapGamma.r));"
                "}"
                "half3 H = (S.rgb + epsilonBase.rgb) * exp(L * W) - epsilonOther.rgb;"
                "return half4(H.r, H.g, H.b, S.a);"
            "} else {"
                "half3 L;"
                "if (isApple == 1) {"
                    "L = pow(G.rgb, half3(appleG));"
                    "L = log(half3(1.0) + (appleH - 1.0) * L);"
                "} else if (noGamma == 1) {"
                    "L = mix(logRatioMin.rgb, logRatioMax.rgb, G.rgb);"
                "} else {"
                    "L = mix(logRatioMin.rgb, logRatioMax.rgb, pow(G.rgb, gainmapGamma.rgb));"
                "}"
                "half3 H = (S.rgb + epsilonBase.rgb) * exp(L * W) - epsilonOther.rgb;"
                "return half4(H.r, H.g, H.b, S.a);"
            "}"
        "}";

static sk_sp<SkRuntimeEffect> gainmap_apply_effect() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForShader(SkString(gGainmapSKSL), {}).effect.release();
    SkASSERT(effect);
    return sk_ref_sp(effect);
}

static bool all_channels_equal(const SkColor4f& c) {
    return c.fR == c.fG && c.fR == c.fB;
}

sk_sp<SkShader> SkGainmapShader::Make(const sk_sp<const SkImage>& baseImage,
                                      const SkRect& baseRect,
                                      const SkSamplingOptions& baseSamplingOptions,
                                      const sk_sp<const SkImage>& gainmapImage,
                                      const SkRect& gainmapRect,
                                      const SkSamplingOptions& gainmapSamplingOptions,
                                      const SkGainmapInfo& gainmapInfo,
                                      const SkRect& dstRect,
                                      float dstHdrRatio,
                                      sk_sp<SkColorSpace> dstColorSpace) {
    return Make(baseImage, baseRect, baseSamplingOptions, gainmapImage, gainmapRect,
                gainmapSamplingOptions, gainmapInfo, dstRect, dstHdrRatio);
}

sk_sp<SkShader> SkGainmapShader::Make(const sk_sp<const SkImage>& baseImage,
                                      const SkRect& baseRect,
                                      const SkSamplingOptions& baseSamplingOptions,
                                      const sk_sp<const SkImage>& gainmapImage,
                                      const SkRect& gainmapRect,
                                      const SkSamplingOptions& gainmapSamplingOptions,
                                      const SkGainmapInfo& gainmapInfo,
                                      const SkRect& dstRect,
                                      float dstHdrRatio) {
    sk_sp<SkColorSpace> baseColorSpace =
            baseImage->colorSpace() ? baseImage->refColorSpace() : SkColorSpace::MakeSRGB();

    // Determine the color space in which the gainmap math is to be applied.
    sk_sp<SkColorSpace> gainmapMathColorSpace =
            gainmapInfo.fGainmapMathColorSpace
                    ? gainmapInfo.fGainmapMathColorSpace->makeLinearGamma()
                    : baseColorSpace->makeLinearGamma();

    // Compute the sampling transformation matrices.
    const SkMatrix baseRectToDstRect = SkMatrix::RectToRectOrIdentity(baseRect, dstRect);
    const SkMatrix gainmapRectToDstRect = SkMatrix::RectToRectOrIdentity(gainmapRect, dstRect);

    // Compute the weight parameter that will be used to blend between the images.
    float W = 0.f;
    if (dstHdrRatio > gainmapInfo.fDisplayRatioSdr) {
        if (dstHdrRatio < gainmapInfo.fDisplayRatioHdr) {
            W = (std::log(dstHdrRatio) - std::log(gainmapInfo.fDisplayRatioSdr)) /
                (std::log(gainmapInfo.fDisplayRatioHdr) -
                 std::log(gainmapInfo.fDisplayRatioSdr));
        } else {
            W = 1.f;
        }
    }

    const bool baseImageIsHdr = (gainmapInfo.fBaseImageType == SkGainmapInfo::BaseImageType::kHDR);
    if (baseImageIsHdr) {
        W -= 1.f;
    }

    // Return the base image directly if the gainmap will not be applied at all.
    if (W == 0.f) {
        return baseImage->makeShader(baseSamplingOptions, &baseRectToDstRect);
    }

    // The base image will have color space conversion performed.
    auto baseImageShader = baseImage->makeShader(baseSamplingOptions, &baseRectToDstRect);

    // The gainmap image shader will ignore any color space that the gainmap has.
    auto gainmapImageShader =
            gainmapImage->makeRawShader(gainmapSamplingOptions, &gainmapRectToDstRect);

    // Create the shader to apply the gainmap in the gain application color space.
    sk_sp<SkShader> gainmapMathShader;
    {
        SkRuntimeShaderBuilder builder(gainmap_apply_effect());
        const SkColor4f logRatioMin({std::log(gainmapInfo.fGainmapRatioMin.fR),
                                     std::log(gainmapInfo.fGainmapRatioMin.fG),
                                     std::log(gainmapInfo.fGainmapRatioMin.fB),
                                     1.f});
        const SkColor4f logRatioMax({std::log(gainmapInfo.fGainmapRatioMax.fR),
                                     std::log(gainmapInfo.fGainmapRatioMax.fG),
                                     std::log(gainmapInfo.fGainmapRatioMax.fB),
                                     1.f});
        const int noGamma =
            gainmapInfo.fGainmapGamma.fR == 1.f &&
            gainmapInfo.fGainmapGamma.fG == 1.f &&
            gainmapInfo.fGainmapGamma.fB == 1.f;
        const uint32_t colorTypeFlags = SkColorTypeChannelFlags(gainmapImage->colorType());
        const int gainmapIsAlpha = colorTypeFlags == kAlpha_SkColorChannelFlag;
        const int gainmapIsRed = colorTypeFlags == kRed_SkColorChannelFlag;
        const int singleChannel = all_channels_equal(gainmapInfo.fGainmapGamma) &&
                                  all_channels_equal(gainmapInfo.fGainmapRatioMin) &&
                                  all_channels_equal(gainmapInfo.fGainmapRatioMax) &&
                                  (colorTypeFlags == kGray_SkColorChannelFlag ||
                                   colorTypeFlags == kAlpha_SkColorChannelFlag ||
                                   colorTypeFlags == kRed_SkColorChannelFlag);
        const SkColor4f& epsilonBase =
                baseImageIsHdr ? gainmapInfo.fEpsilonHdr : gainmapInfo.fEpsilonSdr;
        const SkColor4f& epsilonOther =
                baseImageIsHdr ? gainmapInfo.fEpsilonSdr : gainmapInfo.fEpsilonHdr;

        const int isApple = gainmapInfo.fType == SkGainmapInfo::Type::kApple;
        const float appleG = 1.961f;
        const float appleH = gainmapInfo.fDisplayRatioHdr;

        builder.child("base") = baseImageShader;
        builder.child("gainmap") = gainmapImageShader;
        builder.uniform("logRatioMin") = logRatioMin;
        builder.uniform("logRatioMax") = logRatioMax;
        builder.uniform("gainmapGamma") = gainmapInfo.fGainmapGamma;
        builder.uniform("epsilonBase") = epsilonBase;
        builder.uniform("epsilonOther") = epsilonOther;
        builder.uniform("noGamma") = noGamma;
        builder.uniform("singleChannel") = singleChannel;
        builder.uniform("gainmapIsAlpha") = gainmapIsAlpha;
        builder.uniform("gainmapIsRed") = gainmapIsRed;
        builder.uniform("W") = W;

        builder.uniform("isApple") = isApple;
        builder.uniform("appleG") = appleG;
        builder.uniform("appleH") = appleH;

        gainmapMathShader = builder.makeShader();
        SkASSERT(gainmapMathShader);
    }

    return gainmapMathShader->makeWithWorkingColorSpace(gainmapMathColorSpace);
}
