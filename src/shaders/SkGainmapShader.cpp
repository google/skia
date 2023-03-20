/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkGainmapShader.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkGainmapInfo.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkImageInfoPriv.h"

#ifdef SK_ENABLE_SKSL
static constexpr char gGainmapSKSL[] =
        "uniform shader base;"
        "uniform shader gainmap;"
        "uniform half4 logRatioMin;"
        "uniform half4 logRatioMax;"
        "uniform half4 gainmapGamma;"
        "uniform half4 epsilonSdr;"
        "uniform half4 epsilonHdr;"
        "uniform half W;"
        "uniform int gainmapIsAlpha;"
        "uniform int gainmapIsRed;"
        "uniform int singleChannel;"
        "uniform int noGamma;"
        ""
        "half4 main(float2 coord) {"
        "    half4 S = base.eval(coord);"
        "    half4 G = gainmap.eval(coord);"
        "    if (gainmapIsAlpha == 1) {"
        "        G = half4(G.a, G.a, G.a, 1.0);"
        "    }"
        "    if (gainmapIsRed == 1) {"
        "        G = half4(G.r, G.r, G.r, 1.0);"
        "    }"
        "    if (singleChannel == 1) {"
        "        half L;"
        "        if (noGamma == 1) {"
        "            L = mix(logRatioMin.r, logRatioMax.r, G.r);"
        "        } else {"
        "            L = mix(logRatioMin.r, logRatioMax.r, pow(G.r, gainmapGamma.r));"
        "        }"
        "        half3 H = (S.rgb + epsilonSdr.rgb) * exp(L * W) - epsilonHdr.rgb;"
        "        return half4(H.r, H.g, H.b, S.a);"
        "    } else {"
        "        half3 L;"
        "        if (noGamma == 1) {"
        "            L = mix(logRatioMin.rgb, logRatioMax.rgb, G.rgb);"
        "        } else {"
        "            L = mix(logRatioMin.rgb, logRatioMax.rgb, pow(G.rgb, gainmapGamma.rgb));"
        "        }"
        "        half3 H = (S.rgb + epsilonSdr.rgb) * exp(L * W) - epsilonHdr.rgb;"
        "        return half4(H.r, H.g, H.b, S.a);"
        "    }"
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
#endif  // SK_ENABLE_SKSL

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
#ifdef SK_ENABLE_SKSL
    sk_sp<SkColorSpace> baseColorSpace =
            baseImage->colorSpace() ? baseImage->refColorSpace() : SkColorSpace::MakeSRGB();

    // Determine the color space in which the gainmap math is to be applied.
    sk_sp<SkColorSpace> gainmapMathColorSpace = baseColorSpace->makeLinearGamma();
    if (!dstColorSpace) {
        dstColorSpace = SkColorSpace::MakeSRGB();
    }

    // Create a color filter to transform from the base image's color space to the color space in
    // which the gainmap is to be applied.
    auto colorXformSdrToGainmap =
            SkColorFilterPriv::MakeColorSpaceXform(baseColorSpace, gainmapMathColorSpace);

    // Create a color filter to transform from the color space in which the gainmap is applied to
    // the destination color space.
    auto colorXformGainmapToDst =
            SkColorFilterPriv::MakeColorSpaceXform(gainmapMathColorSpace, dstColorSpace);

    // The base image shader will convert into the color space in which the gainmap is applied.
    const SkMatrix baseRectToDstRect = SkMatrix::RectToRect(baseRect, dstRect);
    auto baseImageShader = baseImage->makeRawShader(baseSamplingOptions, &baseRectToDstRect)
                                   ->makeWithColorFilter(colorXformSdrToGainmap);

    // The gainmap image shader will ignore any color space that the gainmap has.
    const SkMatrix gainmapRectToDstRect = SkMatrix::RectToRect(gainmapRect, dstRect);
    auto gainmapImageShader =
            gainmapImage->makeRawShader(gainmapSamplingOptions, &gainmapRectToDstRect);

    // Create the shader to apply the gainmap.
    sk_sp<SkShader> gainmapMathShader;
    {
        SkRuntimeShaderBuilder builder(gainmap_apply_effect());
        const SkColor4f logRatioMin({sk_float_log(gainmapInfo.fGainmapRatioMin.fR),
                                     sk_float_log(gainmapInfo.fGainmapRatioMin.fG),
                                     sk_float_log(gainmapInfo.fGainmapRatioMin.fB),
                                     1.f});
        const SkColor4f logRatioMax({sk_float_log(gainmapInfo.fGainmapRatioMax.fR),
                                     sk_float_log(gainmapInfo.fGainmapRatioMax.fG),
                                     sk_float_log(gainmapInfo.fGainmapRatioMax.fB),
                                     1.f});
        const float Wunclamped =
                (sk_float_log(dstHdrRatio) - sk_float_log(gainmapInfo.fDisplayRatioSdr)) /
                (sk_float_log(gainmapInfo.fDisplayRatioHdr) -
                 sk_float_log(gainmapInfo.fDisplayRatioSdr));
        const float W = std::max(std::min(Wunclamped, 1.f), 0.f);
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
        builder.child("base") = baseImageShader;
        builder.child("gainmap") = gainmapImageShader;
        builder.uniform("logRatioMin") = logRatioMin;
        builder.uniform("logRatioMax") = logRatioMax;
        builder.uniform("gainmapGamma") = gainmapInfo.fGainmapGamma;
        builder.uniform("epsilonSdr") = gainmapInfo.fEpsilonSdr;
        builder.uniform("epsilonHdr") = gainmapInfo.fEpsilonHdr;
        builder.uniform("noGamma") = noGamma;
        builder.uniform("singleChannel") = singleChannel;
        builder.uniform("gainmapIsAlpha") = gainmapIsAlpha;
        builder.uniform("gainmapIsRed") = gainmapIsRed;
        builder.uniform("W") = W;
        gainmapMathShader = builder.makeShader();
        SkASSERT(gainmapMathShader);
    }

    // Return a shader that will apply the gainmap and then convert to the destination color space.
    return gainmapMathShader->makeWithColorFilter(colorXformGainmapToDst);
#else
    // This shader is currently only implemented using SkSL.
    return nullptr;
#endif
}
