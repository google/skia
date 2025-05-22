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

// For non-Vulkan configs, these settings cover 49 of the 87 cases in 'kCases'.
// They create 61 Pipelines so only modestly over-generate (12 extra Pipelines - 20%).
//
// For Vulkan configs, the Vulkan-specific PrecompileSettings handle 6 more cases and
// add 8 more Pipelines.
//
// These are sorted into groups based on (first) PaintOptions creation function and
// then Render Pass Properties.
// This helps observe DrawTypeFlags distributions.
const PrecompileSettings kPrecompileCases[] = {
// 100% (1/1) handles: 0
/*  0 */ { ImagePremulHWOnlySrcover(),         DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D },
// 100% (2/2) handles: 23 45
/*  1 */ { ImagePremulHWOnlySrcover(),         kRRectAndNonAARect,              kRGBA_1_D },
// 50% (2/4) handles: 66 75 - due to the w/o msaa load variants not being used
/*  2 */ { ImagePremulHWOnlySrcover(),         kRRectAndNonAARect,              kRGBA_4_DS },

// 100% (1/1) handles: 1
/*  3 */ { ImageHWOnlySRGBSrcover(),           DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D_SRGB },
// 100% (2/2) handles: 25 47
/*  4 */ { ImageHWOnlySRGBSrcover(),           kRRectAndNonAARect,              kRGBA_1_D_SRGB },

// 100% (2/2) handles: 9 29
/*  5 */ { TransparentPaintImagePremulHWOnlySrcover(), kRRectAndNonAARect,      kRGBA_1_D },
// 50% (2/4) handles 64 69 - due to the w/o msaa load variants not being used
/*  6 */ { TransparentPaintImagePremulHWOnlySrcover(), kRRectAndNonAARect,      kRGBA_4_DS },

// 100% (2/2) handles 10 30
/*  7 */ { TransparentPaintImageSRGBHWOnlySrcover(), kRRectAndNonAARect,        kRGBA_1_D_SRGB },

// 75% (2/2) handles 28 59 60
/*  8 */ { SolidSrcSrcover(),                  kRRectAndNonAARect,              kRGBA_1_D },
// 50% (1/2) handles 77 - due to the w/o msaa load variants not being used
/*  9 */ { SolidSrcover(),                     DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },

// 100% (2/2) handles 19 39
/* 10 */ { ImagePremulHWOnlyMatrixCFSrcover(), kRRectAndNonAARect,              kRGBA_1_D },

// 100% (1/1) handles 12
/* 11 */ { TransparentPaintImagePremulHWOnlyMatrixCFSrcover(),
                                               DrawTypeFlags::kAnalyticRRect,   kRGBA_1_D },

// 100% (2/2) handles 14 31
/* 12 */ { TransparentPaintImagePremulHWOnlyMatrixCFDitherSrcover(),
                                               kRRectAndNonAARect,              kRGBA_1_D },
// 50% (1/2) handles 71 - due to the w/o msaa load variants not being used
/* 13 */ { TransparentPaintImagePremulHWOnlyMatrixCFDitherSrcover(),
                                               DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },

// 100% (2/2) handles 16 33
/* 14 */ { ImagePremulHWOnlyMatrixCFDitherSrcover(), kRRectAndNonAARect,        kRGBA_1_D },
// 50% (1/2) handles 72 - due to the w/o msaa load variants not being used
/* 15 */ { ImagePremulHWOnlyMatrixCFDitherSrcover(), DrawTypeFlags::kNonAAFillRect, kRGBA_4_DS },

// 100% (2/2) handles 15 32
/* 16 */ { TransparentPaintImageSRGBHWOnlyMatrixCFDitherSrcover(),
                                               kRRectAndNonAARect,              kRGBA_1_D_SRGB },

// 100% (2/2) handles 17 35
/* 17 */ { ImageSRGBHWOnlyMatrixCFDitherSrcover(), kRRectAndNonAARect,          kRGBA_1_D_SRGB },
// 50% (1/2) handles 80 - due to the w/o msaa load variants not being used
/* 18 */ { ImageSRGBHWOnlyMatrixCFDitherSrcover(), DrawTypeFlags::kAnalyticRRect, kRGBA_4_DS_SRGB },

// 100% (2/2) handles 22 42
/* 19 */ { SolidMatrixCFSrcover(),             kRRectAndNonAARect,              kRGBA_1_D },
// 50% (1/2) handles 83
/* 20 */ { SolidMatrixCFSrcover(),             DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },

// 100% (1/1) handles 37
/* 21 */ { ImageAlphaPremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect, kRGBA_1_D },
// 50% (1/2) handles 73 - due to the w/o msaa load variants not being used
/* 22 */ { ImageAlphaPremulHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect, kRGBA_4_DS },

// 100% (1/1) handles 38
/* 23 */ { ImageAlphaSRGBHWOnlyMatrixCFSrcover(), DrawTypeFlags::kNonAAFillRect,kRGBA_1_D_SRGB },

// 100% (1/1) handles 44
/* 24 */ { ImagePremulHWOnlySrc(),             DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 100% (1/1) handles 62
/* 25 */ { ImagePremulHWOnlySrc(),             DrawTypeFlags::kPerEdgeAAQuad,   kRGBA_1_D },
// 50% (1/2) handles 74 - due to the w/o msaa load variants not being used
/* 26 */ { ImagePremulHWOnlySrc(),             DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },

// 100% (1/1) handles 5
/* 27 */ { MouriMapBlur(),                     DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D },
// 100% (1/1) handles 58
/* 28 */ { MouriMapToneMap(),                  DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D_SRGB },
// 100% (1/1) handles 7
/* 29 */ { MouriMapCrosstalkAndChunk16x16(),   DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D_SRGB },
// 100% (1/1) handles 6
/* 30 */ { MouriMapChunk8x8Effect(),           DrawTypeFlags::kNonAAFillRect,   kRGBA16F_1_D },
// 100% (2/2) handles 55, 56
/* 31 */ { KawaseBlurLowSrcSrcOver(),          DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 100% (1/1) handles 54
/* 32 */ { KawaseBlurHighSrc(),                DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 100% (1/1) handler 52
/* 33 */ { BlurFilterMix(),                    DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },

#if defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)
// 100% (2/2) handles 26 50
/* 34 */ { ImagePremulYCbCr238Srcover(),       kRRectAndNonAARect,              kRGBA_1_D },
// 100% (1/1) handles 49
/* 35 */ { ImagePremulYCbCr240Srcover(),       DrawTypeFlags::kNonAAFillRect,   kRGBA_1_D },
// 50% (1/2) handles 76
/* 36 */ { ImagePremulYCbCr240Srcover(),       DrawTypeFlags::kNonAAFillRect,   kRGBA_4_DS },
// 50% (1/2) handles 70
/* 37 */ { TransparentPaintImagePremulYCbCr240Srcover(), DrawTypeFlags::kNonAAFillRect,kRGBA_4_DS },
// 100% (1/1) handles 8
/* 38 */ { MouriMapCrosstalkAndChunk16x16YCbCr247(),DrawTypeFlags::kNonAAFillRect,kRGBA16F_1_D_SRGB },
#endif
};

//
// These Pipelines are candidates for inclusion in Android's precompile. They were generated
// by collecting all the Pipelines the main Android CUJs, including various HDR cases.
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
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   X */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
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
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  12 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
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
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] Dither ] SrcOver" },
/*  19 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] MatrixColorFilter ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  22 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*  23 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  25 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*  26 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*  28 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "SolidColor SrcOver" },
/*  29 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  30 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  31 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  32 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  33 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*  35 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*  37 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  38 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  39 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver AnalyticClip" },
/*  42 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ SolidColor MatrixColorFilter ] SrcOver AnalyticClip" },
/*  44 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  45 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  47 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  49 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  50 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  52 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_BlurFilterMixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_GainmapEffect [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] ColorSpaceTransformPremul ColorSpaceTransformPremul ] Src" },
/*  54 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_HighSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*  55 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*  56 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_LowSampleBlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransform ColorSpaceTransform ] Src" },
/*  58 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] Src" },
/*  59 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src" },
/*  60 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver AnalyticClip" },
/*  62 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "PerEdgeAAQuadRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*   ? */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*  64 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  66 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[InverseCover] + "
                "(empty)" },
/*  69 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  70 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  71 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  72 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  73 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  74 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  75 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  76 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  77 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "MiddleOutFanRenderStep[EvenOdd] + "
                "(empty)" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateCurvesRenderStep[EvenOdd] + "
                "(empty)" },
/*  80 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[InverseCover] + "
                "(empty)" },
/*   X */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransform ColorSpaceTransform ] Dither ] SrcOver" },
/*  83 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ SolidColor MatrixColorFilter ] SrcOver" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "MiddleOutFanRenderStep[EvenOdd] + "
                "(empty)" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateCurvesRenderStep[EvenOdd] + "
                "(empty)" },
/*   ? */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] Modulate ] Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
};

bool skip(const char* str) {
    if (strstr(str, "AnalyticClip")) {  // we have to think about this a bit more
        return true;
    }
#if !defined(SK_VULKAN)
    if (strstr(str, "HardwareImage(3:")) {
        return true;
    }
#endif // SK_VULKAN
    if (strstr(str, "RE_GainmapEffect")) {
        return true;
    }
    if (strstr(str, "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader")) {
        return true;
    }
    if (strstr(str, "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader")) {
        return true;
    }
    if (strstr(str, "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader")) {
        return true;
    }
    return false;
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

    std::set<int> MSAALoadOnlyCases = { 2, 6, 9, 13, 15, 22, 26, 36, 37 };

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
