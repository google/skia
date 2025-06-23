/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/sksl/SkSLUtil.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

#include <set>

using namespace skgpu::graphite;
using namespace PrecompileTestUtils;

namespace {
constexpr bool kWithAnalyticClip = true;

// For non-Vulkan configs, these settings cover 95 of the 161 cases in 'kCases'.
// They create 126 Pipelines so only modestly over-generate (31 extra Pipelines - 25%).
//
// For Vulkan configs, the Vulkan-specific PrecompileSettings handle 12 more cases and
// add 15 more Pipelines.
//
// These are sorted into groups based on (first) PaintOptions creation function and
// then Render Pass Properties.
// This helps observe DrawTypeFlags distributions.
const PrecompileSettings kPrecompileCases[] = {
// 100% (1/1) handles: 0
/*  0 */ { ImagePremulHWOnlySrcover(),         DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D },
// 100% (4/4) handles: 22 23 42 43
/*  1 */ { ImagePremulHWOnlySrcover(),
           kRRectAndNonAARect,
           kRGBA_1_D,
           kWithAnalyticClip },

// 50% (2/4) handles: 63 72 - due to the w/o msaa load variants not being used
/*  2 */ { ImagePremulHWOnlySrcover(),         kRRectAndNonAARect,              kRGBA_4_DS },

// 100% (1/1) handles: 1
/*  3 */ { ImageHWOnlySRGBSrcover(),           DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D_SRGB },
// 100% (4/4) handles: 24 44 45 110
/*  4 */ { ImageHWOnlySRGBSrcover(),
           kRRectAndNonAARect,
           kRGBA_1_D_SRGB,
           kWithAnalyticClip },

// 100% (4/4) handles: 9 28 95 106
/*  5 */ { TransparentPaintImagePremulHWOnlySrcover(),
           kRRectAndNonAARect,
           kRGBA_1_D,
           kWithAnalyticClip },
// 50% (2/4) handles 61 66 - due to the w/o msaa load variants not being used
/*  6 */ { TransparentPaintImagePremulHWOnlySrcover(), kRRectAndNonAARect,      kRGBA_4_DS },

// 100% (2/2) handles 10 29
/*  7 */ { TransparentPaintImageSRGBHWOnlySrcover(), kRRectAndNonAARect,        kRGBA_1_D_SRGB },

// 62% (5/8) handles 27 56 57 58 94
/*  8 */ { SolidSrcSrcover(),
           kRRectAndNonAARect,
           kRGBA_1_D,
           kWithAnalyticClip },

// 50% (2/4) handles 74 86
/*  9 */ { SolidSrcover(),                     kRRectAndNonAARect,              kRGBA_4_DS },

// 100% (2/2) handles 19 38
/* 10 */ { ImagePremulHWOnlyMatrixCFSrcover(), kRRectAndNonAARect,              kRGBA_1_D },

// 100% (1/1) handles 12
/* 11 */ { TransparentPaintImagePremulHWOnlyMatrixCFSrcover(),
                                               DrawTypeFlags::kAnalyticRRect,   kRGBA_1_D },

// 100% (2/2) handles 14 30
/* 12 */ { TransparentPaintImagePremulHWOnlyMatrixCFDitherSrcover(),
                                               kRRectAndNonAARect,              kRGBA_1_D },
// 50% (1/2) handles 68 - due to the w/o msaa load variants not being used
/* 13 */ { TransparentPaintImagePremulHWOnlyMatrixCFDitherSrcover(),
                                               DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },

// 75% (3/4) handles 16 32 33
/* 14 */ { ImagePremulHWOnlyMatrixCFDitherSrcover(),
           kRRectAndNonAARect,
           kRGBA_1_D,
           kWithAnalyticClip },

// 50% (1/2) handles 69 - due to the w/o msaa load variants not being used
/* 15 */ { ImagePremulHWOnlyMatrixCFDitherSrcover(), DrawTypeFlags::kNonAAFillRect, kRGBA_4_DS },

// 100% (2/2) handles 15 31
/* 16 */ { TransparentPaintImageSRGBHWOnlyMatrixCFDitherSrcover(),
                                               kRRectAndNonAARect,              kRGBA_1_D_SRGB },

// 75% (3/4) handles 17 34 35
/* 17 */ { ImageSRGBHWOnlyMatrixCFDitherSrcover(),
           kRRectAndNonAARect,
           kRGBA_1_D_SRGB,
           kWithAnalyticClip },

// 50% (1/2) handles 77 - due to the w/o msaa load variants not being used
/* 18 */ { ImageSRGBHWOnlyMatrixCFDitherSrcover(), DrawTypeFlags::kAnalyticRRect, kRGBA_4_DS_SRGB },

// 100% (1/1) handles 36
/* 19 */ { ImageAlphaPremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect, kRGBA_1_D },

// 50% (1/2) handles 70 - due to the w/o msaa load variants not being used
/* 20 */ { ImageAlphaPremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect, kRGBA_4_DS },

// 100% (1/1) handles 37
/* 21 */ { ImageAlphaSRGBHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect,kRGBA_1_D_SRGB },

// 100% (2/2) handles 41 100
/* 22 */ { ImagePremulHWOnlySrc(),             kRRectAndNonAARect,              kRGBA_1_D },
// 100% (1/1) handles 59
/* 23 */ { ImagePremulHWOnlySrc(),             DrawTypeFlags::kPerEdgeAAQuad,   kRGBA_1_D },
// 50% (1/2) handles 71 - due to the w/o msaa load variants not being used
/* 24 */ { ImagePremulHWOnlySrc(),             DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },

// 100% (1/1) handles 5
/* 25 */ { MouriMapBlur(),                     DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D },
// 100% (1/1) handles 55
/* 26 */ { MouriMapToneMap(),                  DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D_SRGB },
// 100% (1/1) handles 7
/* 27 */ { MouriMapCrosstalkAndChunk16x16(),   DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D_SRGB },
// 100% (1/1) handles 6
/* 28 */ { MouriMapChunk8x8Effect(),           DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D },
// 100% (2/2) handles 52, 53
/* 29 */ { KawaseBlurLowSrcSrcOver(),          DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 100% (1/1) handles 51
/* 30 */ { KawaseBlurHighSrc(),                DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 100% (2/2) handles 49 99
/* 31 */ { BlurFilterMix(),                    kRRectAndNonAARect,              kRGBA_1_D },

// These two are solid colors drawn w/ a LinearEffect

// 100% (1/1) handles 4
/* 32 */ { LinearEffect("UNKNOWN__SRGB__false__UNKNOWN",
                        ChildType::kSolidColor,
                        SkBlendMode::kSrcOver),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA16F_1_D_SRGB },
// 100% (1/1) handles 54
/* 33 */ { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                        ChildType::kSolidColor,
                        SkBlendMode::kSrc),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA_1_D_SRGB },

// 100% (1/1) handles 2
/* 34 */ { LinearEffect("UNKNOWN__SRGB__false__UNKNOWN",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA16F_1_D_SRGB },

// These next two only differ by dst surface

// 100% (1/1) handles 26
/* 35 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_1_D_SRGB },
// 50% (1/2) handles 64 - due to the w/o msaa load variants not being used
/* 36 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_4_DS_SRGB },

// These next two are the same as the above two but w/ transparent paints

// 100% (1/1) handles 11
/* 37 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ false),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_1_D_SRGB },
// 50% (1/2) handlers 62 - due to the w/o msaa load variants not being used
/* 38 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ false),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_4_DS_SRGB },

// The next 3 have a RE_LinearEffect and a MatrixFilter along w/ different ancillary additions
// 100% (1/1) handles 20
/* 39 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ true,
                        /* matrixColorFilter= */ true),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_1_D_SRGB },
// 100% (1/1) handles 13
/* 40 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ false,
                        /* matrixColorFilter= */ true),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_1_D_SRGB },
// 100% (1/1) handles 18
/* 41 */ { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                        ChildType::kHWTexture,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ true,
                        /* matrixColorFilter= */ true,
                        /* dither= */ true),
           DrawTypeFlags::kAnalyticRRect,
           kRGBA_1_D_SRGB },

// 68% (15/22) handles 65 75 76 78 80 81 83 84 (154 155 156 157 158 159 160)
/* 42 */ { SolidSrcover(), DrawTypeFlags::kNonSimpleShape, kRGBA_4_DS },

// AnalyticClip block - all the PrecompileOptions here are just clones of earlier ones
// with an additional kAnalyticClip flag

// Note: this didn't get folded into #2 since the RRect draw isn't appearing w/ a clip
// 50% (1/2) handles: 91
/* 43 */ { ImagePremulHWOnlySrcover(),
           DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
           kRGBA_4_DS },

// Note: this didn't get folded into #9 since the RRect draw isn't appearing w/ a clip
// 75% (3/4) handles 89 92 144
/* 44 */ { SolidSrcSrcover(),
           DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
           kRGBA_4_DS },

//--------------------------------------------------

// 50% (1/2) handles 60
/* 45 */ { {}, // ignored
           DrawTypeFlags::kDropShadows,
           kRGBA_1_D },

// 75% (3/4) handles 82 85 145
/* 46 */ { {}, // ignored
           DrawTypeFlags::kDropShadows,
           kRGBA_4_DS },

// 100% (1/1) handles 96
/* 47 */ { EdgeExtensionPremulSrcover(),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA_1_D },

// 100% (1/1) handles 97
/* 48 */ { TransparentPaintEdgeExtensionPassthroughSrcover(),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA_1_D },

// 100% (1/1) handles 98
/* 49 */ { TransparentPaintEdgeExtensionPremulSrcover(),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA_1_D },

#if defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)
// 238 block ----------------

// 75% (3/4) handles 25 47 48
/* 50 */ { ImagePremulYCbCr238Srcover(),
           kRRectAndNonAARect,
           kRGBA_1_D,
           kWithAnalyticClip },

// 100% (2/2) handles 93 119
/* 51 */ { TransparentPaintImagePremulYCbCr238Srcover(), kRRectAndNonAARect,    kRGBA_1_D },
// 100% (2/2) handles 87 88
/* 52 */ { ImagePremulYCbCr238Srcover(),       kRRectAndNonAARect,              kRGBA_4_DS },

// Note: this didn't get folded into #52 since the RRect draw isn't appearing w/ a clip
// 100% (1/1) handles 90
/* 53 */ { ImagePremulYCbCr238Srcover(),
           DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
           kRGBA_4_DS },

// 240 block ----------------

// 100% (1/1) handles 46
/* 54 */ { ImagePremulYCbCr240Srcover(),       DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 50% (1/2) handles 73
/* 55 */ { ImagePremulYCbCr240Srcover(),       DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },
// 50% (1/2) handles 67
/* 56 */ { TransparentPaintImagePremulYCbCr240Srcover(), DrawTypeFlags::kNonAAFillRect,kRGBA_4_DS },

// 247 block ----------------

// 100% (1/1) handles 8
/* 57 */ { MouriMapCrosstalkAndChunk16x16YCbCr247(),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA16F_1_D_SRGB },

// The next 2 have the same PaintOptions but different destination surfaces

// 75% (3/4) handles 21 39 40
/* 58 */ { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                        ChildType::kHWTextureYCbCr247,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ true,
                        /* matrixColorFilter= */ false,
                        /* dither= */ true),
           kRRectAndNonAARect,
           kRGBA_1_D_SRGB,
           kWithAnalyticClip },

// 100% (1/1) handles 79
/* 59 */ { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                        ChildType::kHWTextureYCbCr247,
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ true,
                        /* matrixColorFilter= */ false,
                        /* dither= */ true),
           DrawTypeFlags::kNonAAFillRect,
           kRGBA_4_DS_SRGB },
#endif
};

// These Pipelines are candidates for inclusion in Android's precompile. They were generated
// by collecting all the Pipelines from the main Android CUJs, including various HDR cases.
//
// The prefix comment pattern's meaning is:
//   a number - handled by some case in kPrecompileCases
//   a ?      - a candidate for addition to kPrecompileCases
//   an X     - skipped (c.f. skip())
//   blank    - not yet investigated
static const PipelineLabel kCases[] = {
/*   0 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   1 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*   2 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   ? */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   4 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   5 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_BlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*   6 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_Chunk8x8Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*   7 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*   8 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*   9 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  10 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  11 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  12 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*  13 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*  14 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  15 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  16 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  17 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  18 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*  19 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*  20 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] SrcOver" },
/*  21 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  22 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  23 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  24 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*  25 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  26 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*  27 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "SolidColor SrcOver" },
/*  28 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  29 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  30 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  31 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  32 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  33 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*  34 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  35 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*  36 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  37 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  38 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*  39 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  40 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver AnalyticClip" },
/*  41 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  42 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  43 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  44 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*  45 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  46 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  47 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  48 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  49 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_GainmapEffect [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] Src" },
/*  51 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_HighSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*  52 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*  53 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver" },
/*  54 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] Src" },
/*  55 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*  56 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src" },
/*  57 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver" },
/*  58 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver AnalyticClip" },
/*  59 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "PerEdgeAAQuadRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  60 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*  61 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  62 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  63 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  64 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*  65 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[InverseCover] + "
                "(empty)" },
/*  66 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  67 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  68 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  69 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  70 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  71 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  72 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  73 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  74 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver" },
/*  75 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "MiddleOutFanRenderStep[EvenOdd] + "
                "(empty)" },
/*  76 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateCurvesRenderStep[EvenOdd] + "
                "(empty)" },
/*  77 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  78 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[InverseCover] + "
                "(empty)" },
/*  79 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  80 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "MiddleOutFanRenderStep[EvenOdd] + "
                "(empty)" },
/*  81 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateCurvesRenderStep[EvenOdd] + "
                "(empty)" },
/*  82 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },

                // New Cases 6/10/25
/*  83 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateWedgesRenderStep[EvenOdd] + "
                "(empty)" },
/*  84 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[RegularCover] + "
                "SolidColor SrcOver" },
/*  85 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticBlurRenderStep + "
                "SolidColor SrcOver" },
/*  86 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "SolidColor SrcOver" },
/*  87 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  88 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  89 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver AnalyticClip" },
/*  90 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  91 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  92 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src AnalyticClip" },
/*  93 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

/*  94 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src AnalyticClip" },

/*  95 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },

//---------------------------------
          // New Cases 6/17/25
/*  96 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/*  97 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  98 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  99 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/* 100 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },

/*     */ { -1, "RP((R8+D16 x1).a000) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ BlendCompose [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] Src" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_Tonemap [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16 [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/* 106 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] SrcOver" },

/* 110 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ RE_MouriMap_Tonemap [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 119 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ BlendCompose [ RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] SrcOver" },

/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },

/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] Passthrough ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect [ SolidColor ColorSpaceTransform ColorSpaceTransform ] Src" },
//--
/* 144 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src AnalyticClip" },
/* 145 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },

/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "overBoundsRenderStep[RegularCover] + "
                "SolidColor SrcOver AnalyticClip" },

/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },

                // Synthetic placeholders for non-convex draw helpers
/* 154 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "MiddleOutFanRenderStep[Winding] + "
                "(empty)" },
/* 155 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "MiddleOutFanRenderStep[Winding] + "
                "(empty)" },
/* 156 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateCurvesRenderStep[Winding] + "
                "(empty)" },
/* 157 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateCurvesRenderStep[Winding] + "
                "(empty)" },
/* 158 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateWedgesRenderStep[Winding] + "
                "(empty)" },
/* 159 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateWedgesRenderStep[Winding] + "
                "(empty)" },
/* 160 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateWedgesRenderStep[EvenOdd] + "
                "(empty)" },
};

bool skip(const char* str) {
#if !defined(SK_VULKAN)
    if (strstr(str, "HardwareImage(3:")) {
        return true;
    }
#endif // SK_VULKAN
    if (strstr(str, "RE_GainmapEffect")) {
        return true;
    }
    return false;
}

// Find any duplicate Pipeline labels
[[maybe_unused]] void find_duplicates() {
    for (size_t i = 0; i < std::size(kCases); ++i) {
        for (size_t j = i+1; j < std::size(kCases); ++j) {
            if (!strcmp(kCases[j].fString, kCases[i].fString)) {
                SkDebugf("%zu is a duplicate of %zu\n", i, j);
            }
        }
    }
}

// The pipeline strings were created with Android Vulkan but we're going to run the test
// on Dawn Metal and all the Native Vulkan configs
bool is_acceptable_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal ||
           type == skgpu::ContextType::kVulkan;
}

} // anonymous namespace

// This test verifies that for each case in 'kPrecompileCases':
//    1) it covers some pipeline(s) in 'kCases'
//    2) more than 40% of the generated Precompile Pipelines are used (i.e., that over-generation
//        isn't too out of control).
// Optionally, it can also:
//    FINAL_REPORT:   Print out a final report that includes missed cases in 'kCases'
//    PRINT_COVERAGE: list the cases (in 'kCases') that are covered by each 'kPrecompileCases' case
//    PRINT_GENERATED_LABELS: list the Pipeline labels for a specific 'kPrecompileCases' case
// Also of note, the "skip" method documents the Pipelines we're intentionally skipping and why.
DEF_GRAPHITE_TEST_FOR_CONTEXTS(AndroidPrecompileTest, is_acceptable_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    using namespace skgpu::graphite;

    //find_duplicates();

#if defined(SK_VULKAN)
    // Use this call to map back from a HardwareImage sub-string to a VulkanYcbcrConversionInfo
    //Base642YCbCr("kEwAAPcAAAAAAAAA");
#endif

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kBGRA_8888_SkColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 skgpu::Protected::kNo,
                                                                 skgpu::Renderable::kYes);

    const bool msaaSupported =
            caps->msaaRenderToSingleSampledSupport() ||
            caps->isSampleCountSupported(TextureInfoPriv::ViewFormat(textureInfo),
                                         caps->defaultMSAASamplesCount());

    if (!msaaSupported) {
        // The following pipelines rely on having MSAA
        return;
    }

#ifdef SK_ENABLE_VELLO_SHADERS
    if (caps->computeSupport()) {
        // The following pipelines rely on not utilizing Vello
        return;
    }
#endif
    std::set<int> MSAALoadOnlyCases = { 2, 6, 9, 13, 15, 20, 24, 36, 38, 55, 56 };

    PipelineLabelInfoCollector collector({ kCases }, skip);

    static const size_t kChosenCase = -1; // only test this entry in 'kPrecompileCases'
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        if (kChosenCase != -1 && kChosenCase != i) {
            continue;
        }

        if (!caps->loadOpAffectsMSAAPipelines() &&
            MSAALoadOnlyCases.find(i) != MSAALoadOnlyCases.end()) {
            // If "w/ msaa load" strings aren't being generated, cases that only handle Pipeline
            // labels with that sub-string will never be matched.
            continue;
        }

        if (kPrecompileCases[i].fRenderPassProps.fDSFlags == DepthStencilFlags::kDepth &&
            caps->getDepthStencilFormat(DepthStencilFlags::kDepth) != TextureFormat::kD16) {
            // The Pipeline labels in 'kCases' have "D16" for this case (i.e., "D32F" is a
            // fine Depth buffer type but won't match the strings).
            continue;
        }

        SkSpan<const SkBlendMode> blendModes = kPrecompileCases[i].fPaintOptions.getBlendModes();
        bool skip = false;
        for (SkBlendMode bm : blendModes) {
            if (bm == SkBlendMode::kSrc && !caps->shaderCaps()->fDualSourceBlendingSupport) {
                // The Pipeline labels were gathered on a device w/ dual source blending.
                // kSrc blend mode w/o dual source blending can result in a dst read and, thus,
                // break the string matching.
                skip = true;
                break;
            }
        }

        if (skip) {
            continue;
        }

        RunTest(precompileContext.get(), reporter, { kPrecompileCases }, i,
                { kCases },
                &collector);
    }

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.

    collector.finalReport();
#endif
}

#endif // SK_GRAPHITE
