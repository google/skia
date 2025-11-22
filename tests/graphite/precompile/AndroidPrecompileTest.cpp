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

typedef void (*VisitSettingsFunc)(
            skgpu::graphite::PrecompileContext*,
            RuntimeEffectManager& effectManager,
            const std::function<void(skgpu::graphite::PrecompileContext*,
                                     const PrecompileSettings&,
                                     int index)>& func);

void VisitAndroidPrecompileSettings_Old(
            skgpu::graphite::PrecompileContext*,
            RuntimeEffectManager& effectManager,
            const std::function<void(skgpu::graphite::PrecompileContext*,
                                     const PrecompileSettings&,
                                     int index)>& func);

void VisitAndroidPrecompileSettings_Protected(
            skgpu::graphite::PrecompileContext*,
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
static const PipelineLabel kOldLabels[] = {
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
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
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
                "RE_MouriMap_CrossTalkAndChunk16x16Effect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] ] Src" },
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
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver" },
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
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
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
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/*  40 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
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
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  47 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  48 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
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
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
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
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
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
                "Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver" },
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
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  88 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  89 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver AnalyticClip" },
/*  90 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  91 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  92 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src AnalyticClip" },
/*  93 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

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
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_MouriMap_Tonemap [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
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
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 119 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

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
                "Compose [ BlendCompose [ RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0) ] ColorSpaceTransform ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
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
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 135 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 136 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
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
                "RuntimeEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] Passthrough ] ] ] SrcOver" },
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
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 149 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },

/* 150 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 151 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
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
                "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
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

/*
 * A new set of labels from 10/8/25
 * Key:
 *  <index> [P-rotected (<matching-index-in-old-set>)]
 */
static const PipelineLabel kNewLabels[] = {
/*   0 P (63*) */ { 72,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   1 P (61*) */ { 65,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*   2 P (46) */ { 39,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   3 */ { 38,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] SrcOver" },
/*   4 P (71*) */ { 37,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*   5 P (86) */ { 34,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "SolidColor SrcOver" },
/*   6 */ { 31,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "TessellateWedgesRenderStep[EvenOdd] + "
        "(empty)" },
/*   7 P (67*) */ { 29,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },

/*   8 */ { 29,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/*   9 */ { 26,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  10 P (82) */ { 26,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "VerticesRenderStep[TrisColor] + "
        "Compose [ PrimitiveColor Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/*  11 */ { 24,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/*  12 */ { 21,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  13 */ { 20,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  14 */ { 18,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  15 P (91) */ { 17,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  16 */ { 16,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor Src" },
/*  17 */ { 16,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  18 */ { 16,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/*  19 */ { 16,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_BoxShadowEffect SrcOver" },
/*  20 */ { 15,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ SolidColor ] ColorSpaceTransformSRGB ] SrcOver" },
/*  21 */ { 14,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  22 */ { 14,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  23 */ { 13,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  24 */ { 12,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  25 */ { 12,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  26 */ { 11,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  27 */ { 10,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  28 P (146) */ { 10,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/*  29 */ { 9 ,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  30 */ { 9 ,
        "RP((RGBA8+D16 x1).rgba) + "
       "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  31 */ { 8 ,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  32 */ { 8 ,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  33 */ { 7,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  34 */ { 7,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  35 P (89) */ { 7,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor SrcOver AnalyticClip" },
/*  36 */ { 7,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_GainmapEffect [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformPremul ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] Src" },
/*  37 */ { 7,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*  38 */ { 6,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  39 */ { 6,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  40 */ { 6,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  41 */ { 6,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  42 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHoAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  43 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  44 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/*  45 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] Passthrough ] ] ] SrcOver" },
/*  46 P (166 - 69*) */ { 5,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  47 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  48 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  49 P (71) */ { 5,
        "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  50 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  51 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  52 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*  53 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  54 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/*  55 */ { 4,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  56 */ { 4,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  57 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  58 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHIAAPAAAAAAAAAA) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  59 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  60 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] MatrixColorFilter ] Dither ] SrcOver" },
/*  61 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  62 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/*  63 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  64 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver" },
/*  65 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/*  66 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  67 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/*  68 */ { 3,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  69 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ SolidColor ] ColorSpaceTransformSRGB ] SrcOver" },
/* 70 */ { 3,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 71 */ { 3,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor SrcOver" },
/* 72 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 73 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 74 P (28)  */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 75 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 76 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 77 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver AnalyticClip" },
/* 78 */ { 3,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/* 79 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  80 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/*  81 */ { 3,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  82 */ { 3,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  83 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*  84 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  85 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*  86 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  87 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*  88 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  89 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  90 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  91 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
/*  92 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/*  93 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] MatrixColorFilter ] SrcOver" },
/*  94 */ { 2,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  95 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  96 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/*  97 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  98 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/*  99 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 100 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] MatrixColorFilter ] SrcOver" },
/* 101 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_GainmapEffect [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] Src" },
/* 102 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/* 103 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 104 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 105 P (63) */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 106 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ BlendCompose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver AnalyticClip" },
/* 107 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/* 108 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 109 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* 110 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 111 */ { 2,
         "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src" },
/* 112 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* 113 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 114 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 115 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* 116 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 117 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/* 118 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 119 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 120 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/* 121 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 122 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_BoxShadowEffect SrcOver" },
/* 123 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* 124 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver" },
/* 125 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/* 126 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/* 127 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 128 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ SolidColor ] ColorSpaceTransformSRGB ] Src" },
/* 129 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "SolidColor SrcOver AnalyticClip" },
/* 130 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 131 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/* 132 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* 133 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHIAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 134 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver AnalyticClip" },
/* 135 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO0AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 136 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO0AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 137 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/* 138 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHIAAPAAAAAAAAAA) ] Passthrough ] ] ] MatrixColorFilter ] Dither ] SrcOver" },
/* 139 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 140 */ { 1,
        "RP((RGBA16F+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 141 */ { 1,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] Plus ] SrcOver" },
/* 142 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 143 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEsAAPcAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 144 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 145 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/* 146 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/* 147 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/* 148 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/* 149 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "VerticesRenderStep[TrisTexCoords] + "
        "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 150 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 151 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] SrcOver" },
/* 152 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/* 153 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] Plus ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 154 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 155 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] Plus ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 156 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 157 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 158 */ { 1,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/* 159 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/* 160 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 161 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] Plus ] MatrixColorFilter ] Dither ] SrcOver" },
/* 162 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* 163 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver" },
/* 164 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* 165 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor SrcOver" },
/* 166 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHIAAO0AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 167 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 168 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 169 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] SrcOver" },
/* 170 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* 171 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 172 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 173 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHoAAO0AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* 174 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 175 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* 176 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 177 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* 178 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/* 179 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },

/* 180 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/* */ { 1, "RP((RGBA16F+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose [ Compose [ RE_LinearEffect_BT2020_ITU_HLG__DISPLAY_BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] MatrixColorFilter ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_BlurFilter_MixEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA16F+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHMAAPAAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ Compose [ RE_LinearEffect_BT2020_ITU_HLG__DISPLAY_BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + "
"LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEoAAPcAAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ BlendCompose [ SolidColor LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Plus ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEoAAPcAAAAAAAAA) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_BT2020_ITU_HLG__DISPLAY_BT2020__false__UNKNOWN__Shader [ SolidColor ] ColorSpaceTransformSRGB ] Src" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformPremul ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "VerticesRenderStep[TrisColor] + "
        "Compose [ PrimitiveColor Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
/* */ { 1,
      "RP((RGBA8+D16 x1).rgba) + "
      "CoverBoundsRenderStep[NonAAFill] + "
      "Compose [ Compose [ Compose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] Dither ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose [ BlendCompose [ RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] MatrixColorFilter ] Dither ] SrcOver" },
/* */ { 1,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader [ Compose [ RE_MouriMap_TonemapEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] Passthrough ] ] ] ColorSpaceTransformSRGB ] ColorSpaceTransformSRGB ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ Compose [ RE_LinearEffect_BT2020_ITU_HLG__0x9060000__false__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kEwAAPcAAAAAAAAA) ] ColorSpaceTransform ] ] ] ColorSpaceTransformSRGB ] Dither ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ BlendCompose [ Compose [ RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader [ RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ] ColorSpaceTransformSRGB ] AlphaOnlyPaintColor SrcIn ] MatrixColorFilter ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHAAAC8AAAAAAAAA) ] ColorSpaceTransformPremul ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose [ RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] ColorSpaceTransformSRGB ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: gHMAAPAAAAAAAAAA) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(3: kHsAAO4AAAAAAAAA) ] Passthrough ] ] ] SrcOver" },

    // Synthetic copy of label 10 "w/ msaa load"
/* */ { -1,
        "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
        "VerticesRenderStep[TrisColor] + "
        "Compose [ PrimitiveColor Compose [ GaussianColorFilter BlendCompose [ SolidColor Passthrough Modulate ] ] ] SrcOver" },
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
[[maybe_unused]] void find_duplicates(SkSpan<const PipelineLabel> labels) {
    for (size_t i = 0; i < labels.size(); ++i) {
        for (size_t j = i+1; j < labels.size(); ++j) {
            if (!strcmp(labels[j].fString, labels[i].fString)) {
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

void test(skiatest::Reporter* reporter,
          skgpu::graphite::Context* context,
          SkSpan<const PipelineLabel> labels,
          VisitSettingsFunc visitSettings) {
    using namespace skgpu::graphite;

    //find_duplicates(labels);

#if defined(SK_VULKAN)
    // Use this call to map back from a HardwareImage sub-string to a VulkanYcbcrConversionInfo
    //Base642YCbCr("kAwAEPcAAAAAAAAA");
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

    PipelineLabelInfoCollector collector(labels, skip);
    RuntimeEffectManager effectManager;

    (*visitSettings)(
         precompileContext.get(),
         effectManager,
         [&](skgpu::graphite::PrecompileContext* precompileContext,
             const PrecompileSettings& precompileCase,
             int index) {
            const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

            static const int kChosenCase = -1; // only test this entry in 'kPrecompileCases'
            if (kChosenCase != -1 && kChosenCase != index) {
                return;
            }

            if (caps->getDepthStencilFormat(DepthStencilFlags::kDepth) != TextureFormat::kD16) {
                // The Pipeline labels in 'kOldLabels' have "D16" for this case (i.e., "D32F" is a
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

            RunTest(precompileContext, reporter, precompileCase, index, labels, &collector);
        });

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'labels' that
    // were not covered by the PaintOptions.

    collector.finalReport();
#endif
}

} // anonymous namespace

// These tests verify that for each visited PaintOption:
//    1) it covers some pipeline(s) in the provided labels
//    2) more than 40% of the generated Precompile Pipelines are used (i.e., that over-generation
//        isn't too out of control).
// Optionally, it can also:
//    FINAL_REPORT:   Print out a final report that includes missed cases in labels
//    PRINT_COVERAGE: list the cases (in the labels) that are covered by each PaintOptions
//    PRINT_GENERATED_LABELS: list the Pipeline labels for a specific PaintOption
// Also of note, the "skip" method documents the Pipelines we're intentionally skipping and why.
DEF_GRAPHITE_TEST_FOR_CONTEXTS(AndroidPrecompileTest_Old, is_acceptable_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    test(reporter, context, kOldLabels, VisitAndroidPrecompileSettings_Old);
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(AndroidPrecompileTest_Protected, is_acceptable_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    test(reporter, context, kNewLabels, VisitAndroidPrecompileSettings_Protected);
}

#endif // SK_GRAPHITE
