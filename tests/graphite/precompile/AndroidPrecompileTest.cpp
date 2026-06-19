/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/TestOptions.h"

#if defined(SK_GRAPHITE)
#include "include/core/SkData.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

using namespace skgpu::graphite;
using namespace PrecompileTestUtils;

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
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/*   1 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/*   2 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/*   ? */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/*   4 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[SolidColor] SrcOver" },
/*   5 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_BlurEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] Src" },
/*   6 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_Chunk8x8Effect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] Src" },
/*   7 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] Src" },
/*   8 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+Unpremul+PQ+Gamut+sRGB+Premul]] Src" },
/*   9 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  10 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  11 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  12 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/*  13 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/*  14 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  15 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  16 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  17 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  18 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+MatrixColorFilter, Dither] SrcOver" },
/*  19 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/*  20 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+MatrixColorFilter SrcOver" },
/*  21 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha]]+Dither SrcOver" },
/*  22 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/*  23 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  24 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/*  25 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  26 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/*  27 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  28 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  29 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  30 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  31 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  32 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  33 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/*  34 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  35 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/*  36 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn+MatrixColorFilter SrcOver" },
/*  37 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[BlendCompose[CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn]]+MatrixColorFilter SrcOver" },
/*  38 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/*  39 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha]]+Dither SrcOver" },
/*  40 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha]]+Dither SrcOver AnalyticClip" },
/*  41 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },
/*  42 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/*  43 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  44 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/*  45 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver AnalyticClip" },
/*  46 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  47 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  48 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  49 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] Src" },
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_GainmapEffect[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+TF+Gamut+TF+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough], PreAlpha+PostAlpha, PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+TF+Gamut+TF+PostAlpha], PreAlpha+PostAlpha, PreAlpha+PostAlpha] Src" },
// Obsolete - Android will no longer generate
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_HighSampleBlurEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] Src" },
// Obsolete - Android will no longer generate
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_LowSampleBlurEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] Src" },
// Obsolete - Android will no longer generate
/*   X */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_KawaseBlurDualFilter_LowSampleBlurEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver" },
/*  54 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[SolidColor] Src" },
/*  55 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul Src" },
/*  56 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  57 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  58 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  59 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "PerEdgeAAQuadRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },
/*  60 */ { -1, "RP((RGBA8+D16 x1).rgba) + VerticesRenderStep[TrisColor] + PrimitiveColor+GaussianColorFilter+BlendCompose[SolidColor, Passthrough, Modulate] SrcOver" },
/*  61 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  62 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  63 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/*  64 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] SrcOver" },
/*  65 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[InverseCover] + "
                "(empty)" },
/*  66 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  67 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  68 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  69 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  70 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[BlendCompose[CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn]]+MatrixColorFilter SrcOver" },
/*  71 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },
/*  72 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/*  73 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
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
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  78 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[InverseCover] + "
                "(empty)" },
/*  79 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha]]+Dither SrcOver" },
/*  80 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "MiddleOutFanRenderStep[EvenOdd] + "
                "(empty)" },
/*  81 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "TessellateCurvesRenderStep[EvenOdd] + "
                "(empty)" },
/*  82 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "VerticesRenderStep[TrisColor] + "
                "PrimitiveColor+GaussianColorFilter+BlendCompose[SolidColor, Passthrough, Modulate] SrcOver" },

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
/*  86 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  87 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  88 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  89 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  90 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  91 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  92 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src AnalyticClip" },
/*  93 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },

/*  94 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src AnalyticClip" },

/*  95 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },

//---------------------------------
          // New Cases 6/17/25
/*  96 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]] SrcOver" },
/*  97 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  98 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  99 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] Src" },
/* 100 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },

/* 101 */ { -1, "RP((R8+D16 x1).a000) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[BlendCompose[ImageShaderClamp(0)+AlphaOnly, RGBPaintColor, DstIn]] Src" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_Tonemap[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* 103 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/* 104 */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_MouriMap_CrossTalkAndChunk16x16Effect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]] Src" },
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] Src" },
/* 106 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 107 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus] SrcOver" },
/* 108 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+MatrixColorFilter, Dither] SrcOver" },
/* 109 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter SrcOver" },
/* 110 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(3: gHoAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 112 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]] SrcOver" },
/* 113 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/* 114 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_Tonemap[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "AnalyticRRectRenderStep + "
                "RuntimeEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] Src" },
/* 117 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 118 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 119 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 120 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[RuntimeEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 122  */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus] SrcOver AnalyticClip" },
/* 123 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* 124  */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[RuntimeEffect[LocalMatrix[CoordNormalize[HardwareImage(x247 2020+narrow cos cos nearest F rgba cf1lf0)]+PreAlpha+TF+Gamut+TF+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+Dither SrcOver" },
/* 126 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[BlendCompose[CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn]]+MatrixColorFilter, Dither] SrcOver" },
/* 128 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver AnalyticClip" },
/* 129 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter SrcOver" },

/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(3: gHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/* 131 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+MatrixColorFilter SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(3: gHIAAPAAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(3: gHoAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 134 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/* 135 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/* 136 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/* 137 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver" },
/* 138 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver AnalyticClip" },
/* 139 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/* 140 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver AnalyticClip" },
/* 141 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+Passthrough]] SrcOver" },
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RuntimeEffect[SolidColor, PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] Src" },
//--
/* 144 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor Src AnalyticClip" },
/* 145 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
                "VerticesRenderStep[TrisColor] + "
                "PrimitiveColor+Compose[GaussianColorFilter, BlendCompose[SolidColor, Passthrough, Modulate]] SrcOver" },
/* 146 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 148 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 149 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },

/* 150 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/* 151 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[RegularCover] + "
                "SolidColor SrcOver AnalyticClip" },

/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough], PreAlpha+TF+Gamut+TF+PostAlpha, PreAlpha+TF+Gamut+TF+PostAlpha], PreAlpha+TF+Gamut+TF+PostAlpha, PreAlpha+TF+Gamut+TF+PostAlpha] SrcOver" },

/* 154 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_LutEffect[LocalMatrix[CoordNormalize[HardwareImage(x54 2020+full mid mid nearest F rgba cf0lf1)]+Unpremul+PQ+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Gamut+sRGB+Premul]+Unpremul+sRGB+Premul, Dither] SrcOver" },

/* 155 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(x769 709+full mid mid nearest F rgba cf0lf1)]+PreAlpha+PostAlpha] SrcOver" },

/* 156 */ { -1, "RP((RGBA8+D16 x1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x54 2020+full mid mid nearest F rgba cf0lf1)]+Unpremul+PQ+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, Dither] SrcOver" },

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
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 63* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "AnalyticRRectRenderStep + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/* 66* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 67* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 68* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 69* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* 71* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },
/* 72* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/* 74* */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
                "CoverBoundsRenderStep[NonAAFill] + "
                "SolidColor SrcOver" },

/* 173 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* 174 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 175 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver" },

/* 176 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 177 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 178 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/* 179 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver" },
/* 180 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },

/* 181 */ { -1,
        "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/* 182 */ { -1,
        "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 183 */ { -1,
        "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul SrcOver" },
/* 184 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 185 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver" },
/* 186 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter] SrcOver" },
/* 187 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough SrcOver" },
/* 188 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/* 189 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 190 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* 191 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 192 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver" },
/* 193 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver AnalyticClip" },
/* 194 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter] SrcOver" },
/* 195 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter] SrcOver AnalyticClip" },
/* 196 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough SrcOver" },
/* 197 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/* 198 */ { -1,
        "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_GainmapEffect[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Passthrough, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]] Src" },

/* 199 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] SrcOver" },
// A RRect w/ a RRect clip!!?
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] SrcOver AnalyticClip" },
/* 201 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_BT2020_HLG__UNKNOWN__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul Src" },
/* 202 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver" },
/* 203 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 204 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/* 205 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 206 */ { -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/* 207 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 208 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 209 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough SrcOver AnalyticClip" },
/* 210 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 211 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 212 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src AnalyticClip" },
/* 213 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough SrcOver" },

/* 214 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 215 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 216 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
// 182 should go here

/* 217 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 218 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 219 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 220 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
// 16F
/*     */ { -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },

/* 222 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 223 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 224 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
// RRect w/ clip!!!
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 226 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },

/* 227 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 228 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 229 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 230 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },

/* 231 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
// RRect w/ clip!!!
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 233 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 234 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/* 235 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 236 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 237 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 238 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 239 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/* fake */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 241 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 242 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/* 243 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* fake */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 245 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/* 246 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver AnalyticClip" },
// RRect w/ clip!!!
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver AnalyticClip" },

/* 248 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver" },

/* 249 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },

/* 250 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/* 251 */ { -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },

/* 252 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
// RRect w/ clip!!!
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },

/* 254 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },

/* 255 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver AnalyticClip" },
// rrect with clip!!!
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver AnalyticClip" },
/*     */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 258 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver AnalyticClip" },

/* 259 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 260 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 261 */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/* 262 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 263 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 264 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/* 265 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul Src" },

/* 266 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[SolidColor]+Unpremul+sRGB+Premul Src" },
// Odd color space
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[SolidColor]+PreAlpha+PostAlpha Src" },
/* 268 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_BT2020_ITU_PQ__DISPLAY_P3__false__0x90a0000__Shader[SolidColor]+Unpremul+sRGB+Premul Src" },
// Odd man out w/ the clip
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* fake */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[SolidColor]+Unpremul+sRGB+Premul Src" },

// RRect w/ clip!!!
/*     */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter] SrcOver AnalyticClip" },
/* fake */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter] SrcOver" },
/* 273 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter] SrcOver" },

//----

#if 0
/* 192 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver" },
/* 193 */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver AnalyticClip" },
/* 185 */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver" },
#endif

/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, MatrixColorFilter], Dither] SrcOver" },
//----

/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+Dither SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+Dither SrcOver AnalyticClip" },

/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], Plus]]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },

/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },

/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_LutEffect[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+ColorSpaceTransform], Plus], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Gamut+sRGB+Premul]+Unpremul+sRGB+Premul+Dither SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough SrcOver" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul+Dither SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Passthrough, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul+Dither SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul+Dither SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]]+Premul] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], Plus]]+PreAlpha+PostAlpha, MatrixColorFilter] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA16F+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Passthrough]+Unpremul+sRGB+Premul SrcOver" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },


/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]] SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilterV2_UpSampleBlurEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilterV2_QuarterResDownSampleBlurEffect[LocalMatrix[CoordClamp[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]]] Src" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + VerticesRenderStep[TrisColor] + PrimitiveColor+GaussianColorFilter+BlendCompose[SolidColor, Passthrough, Modulate] SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x842094169 709+narrow mid mid nearest F rgba cf0lf1)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_KawaseBlurDualFilterV2_QuarterResDownSampleBlurEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] Src" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] SrcOver" },
// RRect with clip??!!
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + AnalyticRRectRenderStep + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D24_S8 x4->1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + AnalyticRRectRenderStep + RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_GainmapEffect[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Passthrough, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] Src" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix[CoordNormalize[HardwareImage(x842094169 709+narrow mid mid nearest F rgba cf0lf1)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver AnalyticClip" },
/*  */ { -1, "RP((RGBA8+D16 x1).rgba) + CoverBoundsRenderStep[NonAAFill] + RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+MatrixColorFilter, Dither] SrcOver" },
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
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/*   1 P (61*) */ { 65,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*   2 P (46) */ { 39,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*   3 */ { 38,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[BlendCompose[CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn]]+MatrixColorFilter SrcOver" },
/*   4 P (71*) */ { 37,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },
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
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },

/*   8 */ { 29,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/*   9 */ { 26,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[LocalMatrix[BlendCompose[CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn]]+MatrixColorFilter, Dither] SrcOver" },
/*  10 P (82) */ { 26,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "VerticesRenderStep[TrisColor] + "
        "PrimitiveColor+GaussianColorFilter+BlendCompose[SolidColor, Passthrough, Modulate] SrcOver" },
/*  11 */ { 24,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/*  12 */ { 21,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  13 */ { 20,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  14 */ { 18,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  15 P (91) */ { 17,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  16 */ { 16,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor Src" },
/*  17 */ { 16,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  18 */ { 16,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] SrcOver" },
/*  19 */ { 16,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_BoxShadowEffect SrcOver" },
/*  20 */ { 15,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul SrcOver" },
/*  21 */ { 14,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  22 */ { 14,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  23 */ { 13,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  24 */ { 12,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  25 */ { 12,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*  26 */ { 11,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*  27 */ { 10,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  28 P (146) */ { 10,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus] SrcOver" },
/*  29 */ { 9 ,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  30 */ { 9 ,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  31 */ { 8 ,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  32 */ { 8 ,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  33 */ { 7,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHoAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  34 */ { 7,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  35 P (89) */ { 7,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor SrcOver AnalyticClip" },
/*  36 */ { 7,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_GainmapEffect[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Passthrough, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] Src" },
/*  37 */ { 7,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/*  38 */ { 6,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  39 */ { 6,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  40 */ { 6,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  41 */ { 6,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  42 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHoAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  43 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  44 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/*  45 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+Passthrough]] SrcOver" },
/*  46 P (166 - 69*) */ { 5,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/*  47 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  48 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  49 P (71) */ { 5,
        "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] Src" },
/*  50 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  51 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  52 */ { 5,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/*  53 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  54 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver AnalyticClip" },
/*  55 */ { 4,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  56 */ { 4,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  57 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/*  58 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(3: gHIAAPAAAAAAAAAA)]+Passthrough]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  59 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]]+MatrixColorFilter, Dither] SrcOver" },
/*  60 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+MatrixColorFilter, Dither] SrcOver" },
/*  61 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  62 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver" },
/*  63 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  64 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver" },
/*  65 */ { 4,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul]+MatrixColorFilter SrcOver" },
/*  66 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: gHIAAPAAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  67 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/*  68 */ { 3,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  69 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul SrcOver" },
/*  70 */ { 3,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  71 */ { 3,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor SrcOver" },
/*  72 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  73 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  74 P (28) */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  75 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul] SrcOver" },
/*  76 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  77 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus] SrcOver AnalyticClip" },
/*  78 */ { 3,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] SrcOver" },
/*  79 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/*  80 */ { 3,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/*  81 */ { 3,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+ColorSpaceTransform], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  82 */ { 3,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/*  83 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver AnalyticClip" },
/*  84 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  85 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] Src" },
/*  86 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  87 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/*  88 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  89 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  90 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  91 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul+Dither SrcOver AnalyticClip" },
/*  92 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul SrcOver" },
/*  93 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+MatrixColorFilter SrcOver" },
/*  94 */ { 2,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  95 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/*  96 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/*  97 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/*  98 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[Compose[BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/*  99 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 100 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]]+MatrixColorFilter SrcOver" },
/* 101 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_GainmapEffect[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] Src" },
/* 102 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver AnalyticClip" },
/* 103 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul] SrcOver" },
/* 104 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul] SrcOver" },
/* 105 P (63) */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/* 106 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+Dither SrcOver AnalyticClip" },
/* 107 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver AnalyticClip" },
/* 108 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 109 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 110 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver" },
/* 111 */ { 2,
         "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src" },
/* 112 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter], Dither] SrcOver" },
/* 113 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Passthrough]+Unpremul+sRGB+Premul SrcOver" },
/* 114 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver" },
/* 115 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 116 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 117 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 118 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 119 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul+Unpremul+sRGB+Premul] SrcOver" },
/* 120 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* 121 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 122 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_BoxShadowEffect SrcOver" },
/* 123 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 124 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* 125 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* 126 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 127 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 128 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[SolidColor]+Unpremul+sRGB+Premul Src" },
/* 129 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "SolidColor SrcOver AnalyticClip" },
/* 130 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/* 131 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* 132 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 133 */ { 2,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHIAAPAAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 134 */ { 2,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]] SrcOver AnalyticClip" },
/* 135 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHIAAO0AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 136 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHIAAO0AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 137 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+ColorSpaceTransform]]+Unpremul+sRGB+Premul+Dither SrcOver" },
/* 138 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(3: gHIAAPAAAAAAAAAA)]+Passthrough]]+MatrixColorFilter, Dither] SrcOver" },
/* 139 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 140 */ { 1,
        "RP((RGBA16F+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul SrcOver" },
/* 141 */ { 1,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Plus] SrcOver" },
/* 142 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 143 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kEsAAPcAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 144 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 145 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 146 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul+Dither SrcOver" },
/* 147 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/* 148 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* 149 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "VerticesRenderStep[TrisTexCoords] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+BlendCompose[SolidColor, Passthrough, PorterDuffBlender] SrcOver" },
/* 150 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 151 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus] SrcOver" },
/* 152 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/* 153 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], Plus]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 154 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 155 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], Plus]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 156 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 157 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 158 */ { 1,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver" },
/* 159 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 160 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 161 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Plus]+MatrixColorFilter, Dither] SrcOver" },
/* 162 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Gamut+sRGB+Premul]+Unpremul+sRGB+Premul+Unpremul+sRGB+Premul SrcOver" },
/* 163 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver" },
/* 164 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver AnalyticClip" },
/* 165 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor SrcOver" },
/* 166 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHIAAO0AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 167 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 168 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 169 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] SrcOver" },
/* 170 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]]+Unpremul+sRGB+Premul]+Unpremul+sRGB+Premul SrcOver" },
/* 171 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 172 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 173 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHoAAO0AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* 174 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* 175 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 176 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]]]+Unpremul+sRGB+Gamut+sRGB+Premul+Unpremul+sRGB+Premul, AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* 177 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul, MatrixColorFilter] SrcOver" },
/* 178 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Gamut+sRGB+Premul]]+Unpremul+sRGB+Premul SrcOver AnalyticClip" },
/* 179 */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },

/* 180 */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_BoxShadowEffect SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* */ { 1, "RP((RGBA16F+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[Compose[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose[Compose[RE_LinearEffect_BT2020_ITU_HLG__DISPLAY_BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+PreAlpha+TF+Gamut+TF+PostAlpha]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Dither] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus]+MatrixColorFilter SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/**/ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Premul]] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_BlurFilter_MixEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]] Src" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA16F+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[Compose[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+PreAlpha+TF+Gamut+TF+PostAlpha], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[Compose[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kHMAAPAAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x240 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"Compose[Compose[RE_LinearEffect_BT2020_ITU_HLG__DISPLAY_BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+PreAlpha+TF+Gamut+TF+PostAlpha]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Dither] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + CoverBoundsRenderStep[NonAAFill] + "
"LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]] SrcOver" },
/* */ { 1, "RP((RGBA8+D24_S8 x4->1).rgba) + "
"AnalyticRRectRenderStep + "
"Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"AnalyticRRectRenderStep + "
"BlendCompose[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1, "RP((RGBA8+D16 x1).rgba) + "
"CoverBoundsRenderStep[NonAAFill] + "
"BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Dither] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kEoAAPcAAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[BlendCompose[SolidColor, LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], Plus], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(3: kEoAAPcAAAAAAAAA)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_BT2020_ITU_HLG__DISPLAY_BT2020__false__UNKNOWN__Shader[SolidColor], PreAlpha+sRGB+Gamut+sRGB+PostAlpha] Src" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(x238 709+narrow mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[LocalMatrix[BlendCompose[CoordNormalize[HardwareImage(0)]+AlphaOnly, RGBPaintColor, DstIn]]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[Compose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[Compose[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], PreAlpha+PostAlpha]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "VerticesRenderStep[TrisColor] + "
        "PrimitiveColor+GaussianColorFilter+BlendCompose[SolidColor, Passthrough, Modulate] SrcOver" },
/* */ { 1,
      "RP((RGBA8+D16 x1).rgba) + "
      "CoverBoundsRenderStep[NonAAFill] + "
      "Compose[Compose[RE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_BT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+PreAlpha+TF+Gamut+TF+PostAlpha]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Dither] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter, Dither] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_SRGB__SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(x238 709+full mid mid nearest F rgba cf1lf0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "Compose[BlendRE_LinearEffect_0x188a0000__V0_SRGB__true__0x9010000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[Compose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]]+MatrixColorFilter, Dither] SrcOver" },
/* */ { 1,
        "RP((RGBA16F+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_LinearEffect_UNKNOWN__SRGB__false__UNKNOWN__Shader[Compose[RE_MouriMap_TonemapEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul], LocalMatrix[CoordNormalize[HardwareImage(0)]+Passthrough]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], PreAlpha+sRGB+Gamut+sRGB+PostAlpha, PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[Compose[RE_LinearEffect_BT2020_ITU_HLG__0x9060000__false__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(3: kEwAAPcAAAAAAAAA)]+PreAlpha+TF+Gamut+TF+PostAlpha]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], Dither] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[BlendCompose[RE_LinearEffect_0x188a0000__DISPLAY_P3__false__0x90a0000__Shader[RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn]+MatrixColorFilter SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "AnalyticRRectRenderStep + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha]+MatrixColorFilter SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHAAAC8AAAAAAAAA)]+PreAlpha+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "Compose[RE_LinearEffect_V0_SRGB__V0_SRGB__true__UNKNOWN__Shader[LocalMatrix[CoordNormalize[HardwareImage(0)]+Unpremul+sRGB+Premul]], PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D24_S8 x4->1).rgba) + "
        "AnalyticRRectRenderStep + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "BlendCompose[LocalMatrix[CoordNormalize[HardwareImage(0)]+PreAlpha+PostAlpha], AlphaOnlyPaintColor, SrcIn] SrcOver AnalyticClip" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "LocalMatrix[CoordNormalize[HardwareImage(3: gHMAAPAAAAAAAAAA)]+PreAlpha+sRGB+Gamut+sRGB+PostAlpha] SrcOver" },
/* */ { 1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "RE_EdgeExtensionEffect[LocalMatrix[CoordNormalize[HardwareImage(3: kHsAAO4AAAAAAAAA)]+Passthrough]] SrcOver" },

    // Synthetic copy of label 10 "w/ msaa load"
/* */ { -1,
        "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load) + "
        "VerticesRenderStep[TrisColor] + "
        "PrimitiveColor+GaussianColorFilter+BlendCompose[SolidColor, Passthrough, Modulate] SrcOver" },
    // Synthetic copy of label 165 where an opaque color converts to Src
/* */ { -1,
        "RP((RGBA8+D16 x1).rgba) + "
        "CoverBoundsRenderStep[NonAAFill] + "
        "SolidColor Src" },
};

struct CallbackData {
    std::unique_ptr<PrecompileContext> fPrecompileContext;
    skiatest::Reporter* fReporter;
};

void pipeline_callback_containsExternalFormat(void* context,
                                              ContextOptions::PipelineCacheOp op,
                                              const std::string& label,
                                              uint32_t uniqueKeyHash,
                                              bool fromPrecompile,
                                              sk_sp<SkData> serializedKey) {
    if (label.empty() || !serializedKey) {
        return;
    }

    CallbackData* data = static_cast<CallbackData*>(context);

    bool hasInLabel = label.find("HardwareImage(x") != std::string::npos;
    bool hasInKey = data->fPrecompileContext->containsExternalFormat(serializedKey) ==
                                     PrecompileContext::ExternalFormatResult::kHasExternalFormat;
    REPORTER_ASSERT(data->fReporter, hasInLabel == hasInKey,
                    "label %d != key %d", hasInLabel, hasInKey);
}

// The pipeline strings were created with Android Vulkan but we're going to run the test
// on Dawn Metal and all the Native Vulkan configs
bool is_acceptable_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal ||
           type == skgpu::ContextType::kVulkan;
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
    PrecompileTest(reporter, context, kOldLabels, VisitAndroidPrecompileSettings_Old);
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(AndroidPrecompileTest_Protected, is_acceptable_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    PrecompileTest(reporter, context, kNewLabels, VisitAndroidPrecompileSettings_Protected);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(AndroidPrecompileTest_containsExternalFormat,
                                           is_acceptable_context_type,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    using namespace skiatest::graphite;

    CallbackData data;
    data.fReporter = reporter;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &data;
    newOptions.fContextOptions.fPipelineCachingCallback = pipeline_callback_containsExternalFormat;

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(origTestContext->contextType());

    Context* newContext = ctxInfo.fContext;
    data.fPrecompileContext = newContext->makePrecompileContext();

    PrecompileTest(reporter, newContext, kOldLabels, VisitAndroidPrecompileSettings_Old);

    data.fPrecompileContext.reset();
}

#endif // SK_GRAPHITE
