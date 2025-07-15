/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GRAPHITE)
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include "tests/graphite/precompile/PaintOptionsBuilder.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

using namespace skgpu::graphite;
using namespace PaintOptionsUtils;
using namespace PrecompileTestUtils;

constexpr bool kWithAnalyticClip = true;

const RenderPassProperties kCombo_RGBA_1D_4DS[2] = { kRGBA_1_D, kRGBA_4_DS };
const RenderPassProperties kCombo_RGBA_1D_4DS_SRGB[2] = { kRGBA_1_D_SRGB, kRGBA_4_DS_SRGB };
const RenderPassProperties kCombo_RGBA_1D_SRGB_w16F[2] = { kRGBA_1_D_SRGB, kRGBA16F_1_D_SRGB };

void VisitPrecompileSettings(skgpu::graphite::PrecompileContext* precompileContext,
                             const std::function<void(skgpu::graphite::PrecompileContext*,
                                                      const PrecompileSettings&,
                                                      int index)>& func) {

    // For non-Vulkan configs, these settings cover 126 of the 170 cases in 'kCases'.
    // They create 153 Pipelines so only modestly over-generate (27 extra Pipelines - 18%).
    //
    // For Vulkan configs, the Vulkan-specific PrecompileSettings handle 24 more cases and
    // add 29 more Pipelines.
    //
    // These are sorted into groups based on (first) PaintOptions creation function and
    // then Render Pass Properties.
    // This helps observe DrawTypeFlags distributions.
    const PrecompileSettings kPrecompileCases[] = {
        // 100% (1/1) handles: 0
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D },

        // 100% (4/4) handles: 22 23 42 43
        { Builder().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (4/4) handles: 63 72 (+2 matching synthetic)
        { Builder().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 100% (1/1) handles: 1
        { Builder().hwImg(kSRGB).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 100% (4/4) handles: 24 44 45 110
        { Builder().hwImg(kSRGB).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 100% (4/4) handles: 9 28 95 106
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (4/4) handles 61 66 (+2 matching synthetic)
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 100% (2/2) handles 10 29
        { Builder().transparent().hwImg(kSRGB).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 63% (5/8) handles 27 56 57 58 94
        { Builder().src().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 75% (3/4) handles 74 86 (+1 matching synthetic)
        { Builder().srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 10: 75% (3/4) handles 19 38 128
        { Builder().hwImg(kPremul).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 75% (3/4) handles 12 123 124
        { Builder().transparent().hwImg(kPremul).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (2/2) handles 14 30
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 100% (2/2) handles 68 (+1 matching synthetic)
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 75% (3/4) handles 16 32 33
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (2/2) handles 69 (+1 matching synthetic)
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 100% (2/2) handles 15 31
        { Builder().transparent().hwImg(kSRGB).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 75% (3/4) handles 17 34 35
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 50% (1/2) handles 77 - due to the w/o msaa load variants not being used
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS_SRGB },

        // 67% (2/3) handles 37 70 - due to the w/o msaa load variants not being used
        { Builder().hwImg(kAlphaSRGB).matrixCF().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 20: 100% (2/2) handles 41 100
        { Builder().hwImg(kPremul).src(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 100% (1/1) handles 59
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kPerEdgeAAQuad,
          kRGBA_1_D },

        // 100% (2/2) handles 71 (+1 matching synthetic)
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // TODO(b/426601394): Group these paint option settings into a function that accepts an
        // input image color space so that the intermediate linear color spaces adapt correctly.
        // 100% (1/1) handles 5
        { MouriMapBlur(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 100% (1/1) handles 55
        { MouriMapToneMap(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 7
        { MouriMapCrosstalkAndChunk16x16Passthrough(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 100% (1/1) handles 6
        { MouriMapChunk8x8Effect(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 100% (2/2) handles 52 53
        { KawaseBlurLowSrcSrcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (1/1) handles 51
        { KawaseBlurHighSrc(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (2/2) handles 49 99
        { BlurFilterMix(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // These two are solid colors drawn w/ a LinearEffect

        // 30: 100% (1/1) handles 4
        { LinearEffect("UNKNOWN__SRGB__false__UNKNOWN",
                       ChildType::kSolidColor,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 100% (1/1) handles 54
        { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                       ChildType::kSolidColor,
                       SkBlendMode::kSrc),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },

        // 100% (2/2) handles 2 141
        { LinearEffect("UNKNOWN__SRGB__false__UNKNOWN",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_SRGB_w16F },

        // 67% (2/3) handles 26 64 - due to the w/o msaa load variants not being used
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 100% (2/2) handles 139 140
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 67% (2/3) handles 11 62 - due to the w/o msaa load variants not being used
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // The next 3 have a RE_LinearEffect and a MatrixFilter along w/ different ancillary
        // additions
        // 100% (1/1) handles 20
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 13
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 18
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 103
        { LinearEffect("V0_SRGB__V0_SRGB__true__UNKNOWN",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 40: 100% (1/1) handles 114
        { LinearEffect("V0_SRGB__V0_SRGB__true__UNKNOWN",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 108
        { LinearEffect("0x188a0000__V0_SRGB__true__0x9010000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 113
        { LinearEffect("0x188a0000__V0_SRGB__true__0x9010000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 120
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 131
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D_SRGB },

        // 62% (15/24) handles 65 75 76 78 80 81 83 84 (+7 synthetic for non-convex draws)
        { Builder().srcOver(),
          DrawTypeFlags::kNonSimpleShape,
          kRGBA_4_DS },

        // Note: this didn't get folded into #2 since the RRect draw isn't appearing w/ a clip
        // 50% (1/2) handles: 91
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // Note: this didn't get folded into #9 since the RRect draw isn't appearing w/ a clip
        // 75% (3/4) handles 89 92 (144)
        { Builder().src().srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 50% (1/2) handles 60
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_1_D },

        // 75% (3/4) handles 82 85 (145)
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_4_DS },

        // 50: 100% (2/2) handles 96 112
        { EdgeExtensionPremulSrcover(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 100% (1/1) handles: 126
        { TransparentPaintEdgeExtensionPassthroughMatrixCFDitherSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (1/1) handles 97
        { TransparentPaintEdgeExtensionPassthroughSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (1/1) handles 98
        { TransparentPaintEdgeExtensionPremulSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (2/2) handles 137 138
        { EdgeExtensionPassthroughSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 50% (1/2) handles 101
        { Builder().hwImg(kAlpha, kClamp).src(),
          DrawTypeFlags::kNonAAFillRect,
          kR_1_D },

        // 100% (2/2) handles 109 129
        { Builder().hwImg(kSRGB).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 104
        { MouriMapCrosstalkAndChunk16x16Premul(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 67% (2/3) handles 107 146
        { Builder().blend().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS },

        // 100% (1/1) handles 122
        { Builder().blend().srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D },

        // 60: 100% (1/1) handles 117
        { Builder().transparent().blend().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

#if defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)

        // 238 Full range (kHIAAO4AAAAAAAAA) block ----------------

        // (3/4) 134 135 150
        { ImagePremulYCbCr238Srcover(/* narrow= */ false),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS_SRGB,
          kWithAnalyticClip },

        // 238 Narrow range (kHoAAO4AAAAAAAAA) block ----------------

        // 75% (3/4) handles 25 47 48
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 75% (3/4) handles 93 119 148
        { TransparentPaintImagePremulYCbCr238Srcover(),
          kRRectAndNonAARect,
          kCombo_RGBA_1D_4DS },

        // 100% (1/1) handles 149
        { TransparentPaintImagePremulYCbCr238Srcover(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 100% (2/2) handles 87 88
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // Note: this didn't get folded into the above since the RRect draw isn't appearing w/ a
        // clip
        // 100% (1/1) handles 90
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 240 (kHIAAPAAAAAAAAAA) block ----------------

        // 100% (2/2) handles 46 136
        { ImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (2/2) handles (73) 151
        { ImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 67% (2/3) handles 67 118 (164)
        { TransparentPaintImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS },

        // 247 (kEwAAPcAAAAAAAAA) block ----------------

        // 70: 100% (1/1) handles 8
        { MouriMapCrosstalkAndChunk16x16YCbCr247(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // The next 2 have the same PaintOptions but different destination surfaces

        // 75% (3/4) handles 21 39 40
        { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                       ChildType::kHWTextureYCbCr247,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ true),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 100% (1/1) handles 79
        { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                       ChildType::kHWTextureYCbCr247,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ true),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS_SRGB },
#endif
    };

    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        func(precompileContext, kPrecompileCases[i], i);
    }
}

#endif // SK_GRAPHITE
