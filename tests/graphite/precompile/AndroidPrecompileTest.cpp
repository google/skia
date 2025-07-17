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
#include "tests/graphite/precompile/AndroidRuntimeEffectManager.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

using namespace skgpu::graphite;
using namespace PrecompileTestUtils;

void VisitPrecompileSettings(skgpu::graphite::PrecompileContext*,
                             RuntimeEffectManager& effectManager,
                             const std::function<void(skgpu::graphite::PrecompileContext*,
                                                      const PrecompileSettings&,
                                                      int index)>& func);

namespace {

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
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*   ? */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*   4 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*   5 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_BlurEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*   6 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_Chunk8x8Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] Src" },
/*   7 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] Src" },
/*   8 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ] Src" },
/*   9 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  10 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  11 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  12 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*  13 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
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
                "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/*  19 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*  20 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/*  21 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver" },
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
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
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
                "Compose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  37 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*  38 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*  39 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*  40 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
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
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ SolidColor ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Src" },
/*  55 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] ColorSpaceTransformSRGB ] Src" },
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
                "Compose [ PrimitiveColor Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*  61 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  62 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  63 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  64 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
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
                "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
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
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*  80 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "MiddleOutFanRenderStep[EvenOdd] + "
                "(empty)" },
/*  81 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateCurvesRenderStep[EvenOdd] + "
                "(empty)" },
/*  82 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ PrimitiveColor Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },

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

/* 101 */ { -1, "RP((R8+D16 x1).a000) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ BlendCompose [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] Src" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_Tonemap [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 103 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 104 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Src" },
/* 106 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 107 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/* 108 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* 109 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] SrcOver" },

/* 110 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 112 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/* 113 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 114 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ RE_MouriMap_Tonemap [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/* 117 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 118 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 119 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

/* 120 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 122  */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver AnalyticClip" },
/* 123 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/* 124  */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ BlendCompose [ RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/* 126 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] Dither ] SrcOver" },
/* 128 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* 129 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] SrcOver" },

/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/* 131  */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*      */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*      */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 134 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 135 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 136 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 137 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver" },
/* 138 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver AnalyticClip" },
/* 139 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },

/* 140 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/* 141 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] Passthrough ] ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect [ SolidColor ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Src" },
//--
/* 144 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src AnalyticClip" },
/* 145 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "VerticesRenderStep[TrisColor] + "
                "Compose [ PrimitiveColor Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/* 146 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/* 148 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 149 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },

/* 150 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 151 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[RegularCover] + "
                "SolidColor SrcOver AnalyticClip" },

/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransform ColorSpaceTransform ] ColorSpaceTransform ColorSpaceTransform ] SrcOver" },

// Synthetic placeholders for non-convex draw helpers. These labels are generated
// for the kNonSimpleShape DrawTypeFlags but don't appear in the naturally
// generated corpus. W/o them here the utilization check would fail.
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "MiddleOutFanRenderStep[Winding] + "
                "(empty)" },
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "MiddleOutFanRenderStep[Winding] + "
                "(empty)" },
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateCurvesRenderStep[Winding] + "
                "(empty)" },
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateCurvesRenderStep[Winding] + "
                "(empty)" },
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateWedgesRenderStep[Winding] + "
                "(empty)" },
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateWedgesRenderStep[Winding] + "
                "(empty)" },
/* N/A */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "TessellateWedgesRenderStep[EvenOdd] + "
                "(empty)" },

// Synthetic placeholders to manage the unpredictability of the "w/ msaa load"
// string. On a device that doesn't generate labels with the "w/msaa load" string,
// kPrecompileCases which only handle such labels will run afoul of the minimum
// utilization check.
/* 61* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 63* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 66* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 67* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 68* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/* 69* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/* 71* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/* 72* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 74* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver" },
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

    PipelineLabelInfoCollector collector({ kCases }, skip);
    RuntimeEffectManager effectManager;

    VisitPrecompileSettings(
         precompileContext.get(),
         effectManager,
         [&](skgpu::graphite::PrecompileContext* precompileContext,
             const PrecompileSettings& precompileCase,
             int index) {
            const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

            static const size_t kChosenCase = -1; // only test this entry in 'kPrecompileCases'
            if (kChosenCase != -1 && kChosenCase != index) {
                return;
            }

            if (caps->getDepthStencilFormat(DepthStencilFlags::kDepth) != TextureFormat::kD16) {
                // The Pipeline labels in 'kCases' have "D16" for this case (i.e., "D32F" is a
                // fine Depth buffer type but won't match the strings).
                bool skip = false;
                for (const RenderPassProperties& rpp : precompileCase.fRenderPassProps) {
                    if (rpp.fDSFlags == DepthStencilFlags::kDepth) {
                        skip = true;
                    }
                }

                if (skip) {
                    return;
                }
            }

            SkSpan<const SkBlendMode> blendModes = precompileCase.fPaintOptions.getBlendModes();
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
                return;
            }

            RunTest(precompileContext, reporter, precompileCase, index,
                    { kCases },
                    &collector);
        });

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.

    collector.finalReport();
#endif
}

#endif // SK_GRAPHITE
