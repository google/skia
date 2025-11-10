/*
 * Copyright 2024 Google LLC
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
#include "tests/graphite/precompile/PaintOptionsBuilder.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

using namespace skgpu::graphite;
using namespace PaintOptionsUtils;
using namespace PrecompileTestUtils;

// Single sampled R w/ just depth
const RenderPassProperties kR_1_D{DepthStencilFlags::kDepth,
                                kAlpha_8_SkColorType,
                                /* fDstCS= */ nullptr,
                                /* fRequiresMSAA= */ false};

// MSAA R w/ depth and stencil
const RenderPassProperties kR_4_DS{DepthStencilFlags::kDepthStencil,
                                 kAlpha_8_SkColorType,
                                 /* fDstCS= */ nullptr,
                                 /* fRequiresMSAA= */ true};

// Single sampled BGRA w/ just depth
const RenderPassProperties kBGRA_1_D{DepthStencilFlags::kDepth,
                                   kBGRA_8888_SkColorType,
                                   /* fDstCS= */ nullptr,
                                   /* fRequiresMSAA= */ false};

// MSAA BGRA w/ just depth
const RenderPassProperties kBGRA_4_D{DepthStencilFlags::kDepth,
                                   kBGRA_8888_SkColorType,
                                   /* fDstCS= */ nullptr,
                                   /* fRequiresMSAA= */ true};

// MSAA BGRA w/ depth and stencil
const RenderPassProperties kBGRA_4_DS{DepthStencilFlags::kDepthStencil,
                                    kBGRA_8888_SkColorType,
                                    /* fDstCS= */ nullptr,
                                    /* fRequiresMSAA= */ true};

// The same as kBGRA_1_D but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_1_D_SRGB{DepthStencilFlags::kDepth,
                                        kBGRA_8888_SkColorType,
                                        SkColorSpace::MakeSRGB(),
                                        /* fRequiresMSAA= */ false};

// The same as kBGRA_1_D but w/ an Adobe RGB colorSpace
const RenderPassProperties kBGRA_1_D_Adobe{
  DepthStencilFlags::kDepth, kBGRA_8888_SkColorType,
  SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB),
  /* fRequiresMSAA= */ false};

// The same as kBGRA_4_DS but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_4_DS_SRGB{DepthStencilFlags::kDepthStencil,
                                         kBGRA_8888_SkColorType,
                                         SkColorSpace::MakeSRGB(),
                                         /* fRequiresMSAA= */ true};

// The same as kBGRA_4_DS but w/ an Adobe RGB colorSpace
const RenderPassProperties kBGRA_4_DS_Adobe{
  DepthStencilFlags::kDepthStencil, kBGRA_8888_SkColorType,
  SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB),
  /* fRequiresMSAA= */ true};

constexpr DrawTypeFlags kRRectAndNonAARect = static_cast<DrawTypeFlags>(
        DrawTypeFlags::kAnalyticRRect | DrawTypeFlags::kNonAAFillRect);
constexpr DrawTypeFlags kQuadAndNonAARect = static_cast<DrawTypeFlags>(
        DrawTypeFlags::kPerEdgeAAQuad | DrawTypeFlags::kNonAAFillRect);

namespace {
// These settings cover 108 of the 255 cases in 'kCases'.
// They create 136 Pipelines so only modestly over-generate (28 Pipelines).
//
// The order here is:
//    First all the kBitmapText_Mask draws
//    Second  all the kBitmapText_Color draws
// Then we switch to being subdivided by the Render Pass Properties and sorted by
// the name of the PaintOptions creation function.
const PrecompileSettings kPrecompileCases[] = {
//-----------------
/*  0 */ { Builder().srcOver(),
           DrawTypeFlags::kBitmapText_Mask,
           kBGRA_1_D },
//-----------------
/*  1 */ { Builder().srcOver(),
           DrawTypeFlags::kBitmapText_Mask,
           kBGRA_4_DS },
//-----------------
/*  2 */ { Builder().linearGrad(kSmall).srcOver(),
           DrawTypeFlags::kBitmapText_Mask,
           kBGRA_4_DS },

//-----------------
/*  3 */ { Builder().transparent().srcOver(),
           DrawTypeFlags::kBitmapText_Color,
           kBGRA_1_D },
/*  4 */ { Builder().srcOver(),
           DrawTypeFlags::kBitmapText_Color,
           kBGRA_1_D_Adobe },
//-----------------
/*  5 */ { Builder().srcOver(),
           DrawTypeFlags::kBitmapText_Color,
           kBGRA_4_DS_Adobe },

//-----------------
/*  6 */ { Builder().srcOver(),
           kRRectAndNonAARect,
           kR_1_D },
/*  7 */ { Builder().hwImg(kAlpha).srcOver(),
           DrawTypeFlags::kPerEdgeAAQuad,
           kR_1_D },
/*  8 */ { Builder().hwImg(kAlpha, kRepeat).src(),
           DrawTypeFlags::kNonAAFillRect,
           kR_1_D },

//-----------------
/*  9 */ { Builder().hwImg(kPremul, kClamp).dstIn(),
           kQuadAndNonAARect,
           kBGRA_1_D },
/* 10 */ { Builder().hwImg(kPremul).matrixCF().srcOver(),
           DrawTypeFlags::kNonAAFillRect,
           kBGRA_1_D },
/* 11 */ { Builder().hwImg(kPremul).porterDuffCF().srcOver(),
           DrawTypeFlags::kPerEdgeAAQuad,
           kBGRA_1_D },
/* 12 */ { Builder().hwImg(kPremul, kClamp).srcOver(),
           DrawTypeFlags::kAnalyticRRect,
           kBGRA_1_D },
/* 13 */ { Builder().hwImg(kPremul).src().srcOver(),
           kQuadAndNonAARect,
           kBGRA_1_D },
/* 14 */ { Builder().linearGrad(kSmall).srcOver(),
           DrawTypeFlags::kNonAAFillRect,
           kBGRA_1_D },
/* 15 */ { Builder().src().srcOver(),
           DrawTypeFlags::kSimpleShape,
           kBGRA_1_D },
/* 16 */ { Builder().transparent().hwImg(kPremul, kClamp).srcOver(),
           kQuadAndNonAARect,
           kBGRA_1_D },
/* 17 */ { Builder().linearGrad(kComplex).dither().srcOver(),
           kRRectAndNonAARect,
           kBGRA_1_D_Adobe },
/* 18 */ { Builder().hwImg(kSRGB).srcOver(),
           kRRectAndNonAARect,
           kBGRA_1_D_SRGB },
/* 19 */ { Builder().hwImg(kSRGB).src(),
           kQuadAndNonAARect,
           kBGRA_1_D_SRGB },
/* 20 */ { Builder().yuv(kNoCubic).srcOver(),
           DrawTypeFlags::kSimpleShape,
           kBGRA_1_D_SRGB },

//-----------------
/* 21 */ { Builder().hwImg(kPremul).dstIn(),
           DrawTypeFlags::kPerEdgeAAQuad,
           kBGRA_4_DS },
/* 22 */ { Builder().hwImg(kPremul).matrixCF().srcOver(),
           DrawTypeFlags::kNonAAFillRect,
           kBGRA_4_DS },
/* 23 */ { Builder().hwImg(kPremul, kClamp).srcOver(),
           kQuadAndNonAARect,
           kBGRA_4_DS },
/* 24 */ { Builder().clear().src().srcOver(),
           kRRectAndNonAARect,
           kBGRA_4_DS },
/* 25 */ { Builder().srcOver(),
           DrawTypeFlags::kNonSimpleShape,
           kBGRA_4_DS },
/* 26 */ { Builder().transparent().hwImg(kPremul).srcOver(),
           DrawTypeFlags::kPerEdgeAAQuad,
           kBGRA_4_DS },
/* 27 */ { Builder().linearGrad(kComplex).dither().srcOver(),
           kRRectAndNonAARect,
           kBGRA_4_DS_Adobe },
/* 28 */ { Builder().hwImg(kSRGB).srcOver(),
           DrawTypeFlags::kAnalyticRRect,
           kBGRA_4_DS_SRGB },
/* 29 */ { Builder().yuv(kHWAndShader).srcOver(),
           DrawTypeFlags::kSimpleShape,
           kBGRA_4_DS_SRGB },
};

// Case 33 is the only case that solely covers Pipeline labels with the "w/ msaa load" sub-string.
#define MSAA_ONLY_CASE 28

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const PipelineLabel kCases[] = {
//--------
/*   0 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Winding] + "
               "(empty)" },
/*   1 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + "
               "(empty)" },
/*   2 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Convex] + "
               "SolidColor SrcOver" },
/*   3 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   4 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "SolidColor SrcOver" },
/*   X */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver AnalyticClip" },
/*   6 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/*   7 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Src" },
/*   8 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Clear" },
/*   ? */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[InverseCover] + "
               "(empty)" },
/*  10 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "SolidColor SrcOver" },
/*  11 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver" },
//--------
/*   ? */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "                       //-----------------------------
               "TessellateWedgesRenderStep[Convex] + "
               "SolidColor SrcOver" },
/*   ? */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "                       //-----------------------------
               "TessellateStrokesRenderStep + "
               "SolidColor SrcOver" },
/*   X */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
//--------
/*  15 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Winding] + "
               "(empty)" },
/*  16 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  17 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "SolidColor SrcOver" },
/*   X */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver AnalyticClip" },
/*  19 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/*  20 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Src" },
/*  21 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Clear" },
/*  22 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   ? */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "       //-----------------------------
               "CoverBoundsRenderStep[InverseCover] + "
               "(empty)" },
/*  24 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Mask] + "
               "SolidColor SrcOver" },
/*  25 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver" },
//--------
/*  26 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] Src" },
/*  27 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  28 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/*  29 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Src" },
/*   X */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  31 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "SolidColor SrcOver" },
/*  32 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver" },
/*   X */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticBlurRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
//--------
/*   X */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   X */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticBlurRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
//--------
/*  36 */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[EvenOdd] + "
               "(empty)" },
/*  37 */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + "
               "SolidColor SrcOver" },
/*   X */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
//--------
/*   X */ { 7, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver AnalyticClip" },
/*  40 */ { 7, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] Src" },
//--------
/*   X */ { 6, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  42 */ { 6, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Mask] + "
               "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/*  43 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  44 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/*   X */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*   X */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  47 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  48 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Src" },
/*  49 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticBlurRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
//--------
/*  51 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  52 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/*   X */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
//--------
/*  54 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  55 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  56 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] DstIn" },
/*  57 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  58 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
//--------
/*  59 */ { 4, "RP((R8+D16 x1).a000) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] SrcOver" },
/*  60 */ { 4, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/*   X */ { 4, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] ] Src" },
/*  62 */ { 4, "RP((R8+D16 x1).a000) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver" },
//--------
/*  63 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + "
               "(empty)" },
/*   X */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  65 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  66 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + "
               "(empty)" },
/*  67 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  68 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*  69 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[InverseCover] + "
               "SolidColor SrcOver" },
/*   X */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
/*  71 */ { 4, "RP((BGRA8+D16 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "SolidColor SrcOver" },
/*  72 */ { 4, "RP((BGRA8+D16 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver" },
//--------
/*  73 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateCurvesRenderStep[EvenOdd] + "
               "(empty)" },
/*   X */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  75 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  76 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  77 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  78 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*  79 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "MiddleOutFanRenderStep[EvenOdd] + "
               "(empty)" },
/*  80 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*  81 */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 4, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticBlurRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
//--------
/*   ? */ { 4, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "          //-----------------------------
               "TessellateWedgesRenderStep[Convex] + "
               "SolidColor SrcOver" },
/*  84 */ { 4, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Mask] + "
               "SolidColor SrcOver" },
//--------
/*  85 */ { 4, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "SolidColor SrcOver" },
/*   X */ { 4, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*   X */ { 4, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] KnownRuntimeEffect_Luma ] SrcOver" },
/*  88 */ { 4, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/*   X */ { 4, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/*   X */ { 4, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
//--------
/*  91 */ { 3, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ BlendCompose [ Compose [ Image(0) ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] Src" },
/*   X */ { 3, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] Src" },
//--------
/*   ? */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "TessellateWedgesRenderStep[Convex] + "
               "(empty)" },
/*  94 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateStrokesRenderStep + "
               "SolidColor SrcOver" },
/*   X */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/*  96 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  98 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] DstIn" },
/*   X */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "SolidColor SrcOver AnalyticClip" },
/*   ? */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/* 101 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 102 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "SolidColor SrcOver AnalyticClip" },
//--------
/* 104 */ { 3, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/* 105 */ { 3, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Src" },
//--------
/*   ? */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "       //-----------------------------
               "TessellateWedgesRenderStep[Convex] + "
               "(empty)" },
/*   X */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   ? */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "       //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/* 109 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverageMaskRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 111 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/* 112 */ { 3, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
//--------
/* 113 */ { 3, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/* 114 */ { 3, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver" },
//--------
/*   X */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 116 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 117 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 118 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
/* 119 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 120 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "CoverageMaskRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 122 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/* 123 */ { 3, "RP((BGRA8+D16 x1).rgba) + "                          //-----------------------------
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/* 124 */ { 2, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] RGBPaintColor DstIn ] ] Src" },
/*   X */ { 2, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur4 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] ] Src" },
/*   X */ { 2, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur4 [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] ] Src" },
//--------
/*   X */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Convex] + "
               "SolidColor SrcOver AnalyticClip" },
/* 128 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*   X */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 130 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*   X */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverageMaskRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 132 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 133 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/* 134 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
//--------
/*   ? */ { 2, "RP((BGRA8+D16 x4->1).rgba) + "                       //-----------------------------
               "TessellateWedgesRenderStep[Convex] + "
               "(empty)" },
/* 136 */ { 2, "RP((BGRA8+D16 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] DstIn" },
//--------
/* 137 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateStrokesRenderStep + "
               "SolidColor SrcOver" },
/*   X */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "SolidColor SrcOver AnalyticClip" },
/*   X */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 140 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/* 141 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/* 142 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Mask] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 143 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/* 144 */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "       //-----------------------------
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
//--------
/*   X */ { 2, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver AnalyticClip" },
/* 146 */ { 2, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Src" },
//--------
/* 147 */ { 2, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*   X */ { 2, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 149 */ { 2, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*   ? */ { 2, "RP((BGRA8+D16 x1).rgba) + "                          //-----------------------------
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 2, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] ] Src" },
/*  152 */ { 2, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*  153 */ { 2, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
//--------
/*   ? */ { 1, "RP((R8+D24_S8 x4->1).a000) + "                       //-----------------------------
               "TessellateWedgesRenderStep[EvenOdd] + "
               "(empty)" },
/*   ? */ { 1, "RP((R8+D24_S8 x4->1).a000) + "                       //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "SolidColor SrcOver" },
//--------
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateStrokesRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] Multiply" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] Multiply" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ BlendCompose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*   ? */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ BlendCompose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 169 */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* 171 */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ RadialGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/* 179 */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
//--------
/*   X */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 182 */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "AnalyticBlurRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + "
               "SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateStrokesRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ BlendCompose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + "
               "Compose [ BlendCompose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 198 */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverageMaskRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 203 */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ RadialGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ RadialGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + "
               "(empty)" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateStrokesRenderStep + "
               "SolidColor SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 215 */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/* 217 */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "SolidColor SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] DstIn" },
/*   X */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] KnownRuntimeEffect_Luma ] SrcOver AnalyticClip" },
/*   X */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor Src AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] DstIn" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur8 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Compose [ MatrixColorFilter MatrixColorFilter ] ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/* 242 */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ BlendCompose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] AlphaOnlyPaintColor SrcIn ] Dither ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "BlendCompose [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] PorterDuffBlender ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "BlendCompose [ Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] PorterDuffBlender ] Src" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverageMaskRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CircularArcRenderStep + "
               "SolidColor SrcOver" },
/*   X */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 250 */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 252 */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*   X */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
};

bool skip(const char* str) {
    if (strstr(str, "AnalyticClip")) {  // we have to think about this a bit more
        return true;
    }
    if (strstr(str, "AnalyticBlurRenderStep")) { // currently internal only
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_1DBlur4")) {  // we have to revise how we do blurring
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_1DBlur16")) {  // we have to revise how we do blurring
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_Luma")) {  // this also seems too specialized
        return true;
    }

    return false;
}

// The pipeline strings were created using the Dawn Metal backend so that is the only viable
// comparison
bool is_dawn_metal_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal;
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
DEF_GRAPHITE_TEST_FOR_CONTEXTS(ChromePrecompileTest, is_dawn_metal_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    using namespace skgpu::graphite;

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

    static const size_t kChosenCase = -1; // only test this entry in 'kPrecompileCases'
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        if (kChosenCase != -1 && kChosenCase != i) {
            continue;
        }

        if (i == MSAA_ONLY_CASE && !caps->loadOpAffectsMSAAPipelines()) {
            // If "w/ msaa load" strings aren't being generated, cases that only handle Pipeline
            // labels with that sub-string will never be matched.
            continue;
        }

        RunTest(precompileContext.get(), reporter, kPrecompileCases[i], i,
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
