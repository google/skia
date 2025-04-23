/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "tools/graphite/UniqueKeyUtils.h"

#include <cstring>
#include <set>

// Print out a final report that includes missed cases in 'kCases'
//#define FINAL_REPORT

// Print out the cases (in 'kCases') that are covered by each 'kPrecompileCases' case
// Also lists the utilization of each 'kPrecompileCases' case
//#define PRINT_COVERAGE

// Print out all the generated labels and whether they were found in 'kCases'.
// This is usually used along with the 'kChosenCase' variable.
//#define PRINT_GENERATED_LABELS

/*** From here to the matching banner can be cut and pasted into Chrome's graphite_precompile.cc **/
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace {

using ::skgpu::graphite::DepthStencilFlags;
using ::skgpu::graphite::DrawTypeFlags;
using ::skgpu::graphite::PaintOptions;
using ::skgpu::graphite::RenderPassProperties;

// "SolidColor SrcOver"
PaintOptions solid_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "SolidColor SrcOver"
// "SolidColor Src"
// "SolidColor Clear"
PaintOptions solid_clear_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kClear,
                                 SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver"
// "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver"
PaintOptions image_premul_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
// LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
PaintOptions yuv_image_srgb_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::YUVImage(
            { &ci, 1 },
            /* includeCubic= */ false) });   // using cubic sampling w/ YUV images is rare
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src"
// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver"
// "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver"
PaintOptions image_premul_src_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src"
PaintOptions image_srgb_src() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                           SkNamedGamut::kAdobeRGB) };
    PaintOptions paintOptions;
    paintOptions.setShaders({ skgpu::graphite::PrecompileShaders::Image({ &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver"
PaintOptions blend_porter_duff_cf_srcover() {
    PaintOptions paintOptions;
    // kSrcOver will trigger the PorterDuffBlender
    paintOptions.setColorFilters(
            { skgpu::graphite::PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

// "RP((R8+D16 x1).a000)"
// Single sampled R w/ just depth
const RenderPassProperties kR_1_D { DepthStencilFlags::kDepth,
                                    kAlpha_8_SkColorType,
                                    /* fDstCS= */ nullptr,
                                    /* fRequiresMSAA= */ false };

// "RP((R8+D24_S8 x4->1).a000)"
// "RP((R8+D24_S8 x4->1).a000 w/ msaa load)"
// MSAA R w/ depth and stencil
const RenderPassProperties kR_4_DS { DepthStencilFlags::kDepthStencil,
                                     kAlpha_8_SkColorType,
                                     /* fDstCS= */ nullptr,
                                     /* fRequiresMSAA= */ true };

// "RP((BGRA8+D16 x1).rgba)"
// Single sampled BGRA w/ just depth
const RenderPassProperties kBGRA_1_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ false };

// "RP((BGRA8+D16 x4->1).rgba)"
// "RP((BGRA8+D16 x4->1).rgba w/ msaa load)"
// MSAA BGRA w/ just depth
const RenderPassProperties kBGRA_4_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ true };

// "RP((BGRA8+D24_S8 x4->1).rgba)"
// "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load)"
// MSAA BGRA w/ depth and stencil
const RenderPassProperties kBGRA_4_DS { DepthStencilFlags::kDepthStencil,
                                        kBGRA_8888_SkColorType,
                                        /* fDstCS= */ nullptr,
                                        /* fRequiresMSAA= */ true };

// The same as kBGRA_1_D but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_1_D_SRGB { DepthStencilFlags::kDepth,
                                            kBGRA_8888_SkColorType,
                                            SkColorSpace::MakeSRGB(),
                                            /* fRequiresMSAA= */ false };

// The same as kBGRA_4_DS but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_4_DS_SRGB { DepthStencilFlags::kDepthStencil,
                                             kBGRA_8888_SkColorType,
                                             SkColorSpace::MakeSRGB(),
                                             /* fRequiresMSAA= */ true };

// These settings cover 176 of the 202 cases in 'kCases'.
const struct PrecompileSettings {
    PaintOptions fPaintOptions;
    DrawTypeFlags fDrawTypeFlags = DrawTypeFlags::kNone;
    RenderPassProperties fRenderPassProps;
} kPrecompileCases[] = {
    { solid_srcover(),                DrawTypeFlags::kSimpleShape,     kR_1_D },

    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kR_4_DS },

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_1_D },
    { blend_porter_duff_cf_srcover(), DrawTypeFlags::kNonSimpleShape,  kBGRA_1_D },
    { image_premul_src_srcover(),     DrawTypeFlags::kSimpleShape,     kBGRA_1_D },
    { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_1_D },

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_4_D },
    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kBGRA_4_D },

    { solid_srcover(),                DrawTypeFlags::kBitmapText_Mask, kBGRA_4_DS },
    { solid_srcover(),                DrawTypeFlags::kCircularArc,     kBGRA_4_DS },
    { solid_srcover(),                DrawTypeFlags::kNonSimpleShape,  kBGRA_4_DS },
    { image_premul_srcover(),         DrawTypeFlags::kSimpleShape,     kBGRA_4_DS },
    { solid_clear_src_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_4_DS },

    { image_srgb_src(),               DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB },
    { yuv_image_srgb_srcover(),       DrawTypeFlags::kSimpleShape,     kBGRA_1_D_SRGB },

    // These two are interesting but have < 40% utility
    // { yuv_image_srgb_srcover(),      DrawTypeFlags::kSimpleShape,     kBGRA_4_DS_SRGB },
    // { solid_srcover(),               DrawTypeFlags::kSimpleShape,     kBGRA_4_D },
};

/*********** Here ends the part that can be pasted into Chrome's graphite_precompile.cc ***********/

// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
[[maybe_unused]] RenderPassProperties get_render_pass_properties(const char* str) {
    static const struct {
        RenderPassProperties fRenderPassProperties;
        const char* fStr;
    } kRenderPassPropertiesMapping[] = {
        { kR_1_D,     "RP((R8+D16 x1).a000)" },
        { kBGRA_1_D,  "RP((BGRA8+D16 x1).rgba)" },
        // These RPPs can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { kR_4_DS,    "RP((R8+D24_S8 x4->1).a000)" },
        { kR_4_DS,    "RP((R8+D24_S8 x4->1).a000 w/ msaa load)" },
        { kBGRA_4_D,  "RP((BGRA8+D16 x4->1).rgba)" },
        { kBGRA_4_D,  "RP((BGRA8+D16 x4->1).rgba w/ msaa load)" },
        { kBGRA_4_DS, "RP((BGRA8+D24_S8 x4->1).rgba)" },
        { kBGRA_4_DS, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load)" },
    };

    for (const auto& rppm : kRenderPassPropertiesMapping) {
        if (strstr(str, rppm.fStr)) {
            return rppm.fRenderPassProperties;
        }
    }

    SkAssertResult(0);
    return {};
}

// This helper maps from the RenderStep's name in the Pipeline label to the DrawTypeFlag that
// resulted in its use.
[[maybe_unused]] DrawTypeFlags get_draw_type_flags(const char* str) {
    static const struct {
        const char* fStr;
        DrawTypeFlags fFlags;
    } kDrawTypeFlagsMapping[] = {
        { "BitmapTextRenderStep[Mask]",                  DrawTypeFlags::kBitmapText_Mask  },
        { "BitmapTextRenderStep[LCD]",                   DrawTypeFlags::kBitmapText_LCD   },
        { "BitmapTextRenderStep[Color]",                 DrawTypeFlags::kBitmapText_Color },

        { "SDFTextRenderStep",                           DrawTypeFlags::kSDFText      },
        { "SDFTextLCDRenderStep",                        DrawTypeFlags::kSDFText_LCD  },

        { "VerticesRenderStep[Tris]",                    DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisTexCoords]",           DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColor]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColorTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[Tristrips]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColor]",          DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColorTexCoords]", DrawTypeFlags::kDrawVertices },

        { "CircularArcRenderStep",                       DrawTypeFlags::kCircularArc  },

        { "AnalyticRRectRenderStep",                     DrawTypeFlags::kAnalyticRRect  },
        { "CoverBoundsRenderStep[NonAAFill]",            DrawTypeFlags::kNonAAFillRect  },
        { "PerEdgeAAQuadRenderStep",                     DrawTypeFlags::kPerEdgeAAQuad  },

        { "CoverageMaskRenderStep",                      DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[RegularCover]",         DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[InverseCover]",         DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[EvenOdd]",             DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[Winding]",             DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateStrokesRenderStep",                 DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Convex]",          DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
    };

    for (const auto& dtfm : kDrawTypeFlagsMapping) {
        if (strstr(str, dtfm.fStr)) {
            SkAssertResult(dtfm.fFlags != DrawTypeFlags::kNone);
            return dtfm.fFlags;
        }
    }

    SkAssertResult(0);
    return DrawTypeFlags::kNone;
}


struct ChromePipeline {
    int fNumHits;         // the number of uses in the top 9 most visited web sites
    const char* fString;
};

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const ChromePipeline kCases[] = {
//--------
/*   0 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*   1 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*   2 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*   3 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*   4 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*   5 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*   6 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*   7 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*   8 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*   9 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  10 */ { 9, "RP((R8+D24_S8 x4->1).a000) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
//--------
/*  11 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  12 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  13 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  14 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  15 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  16 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  17 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  18 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  19 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  20 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  21 */ { 9, "RP((R8+D24_S8 x4->1).a000 w/ msaa load) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
//--------
/*  22 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  23 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  24 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  25 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  26 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  27 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  28 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/*  29 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Src" },
/*  30 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Clear" },
/*  31 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  32 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  33 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  34 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  35 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  36 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  37 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  38 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  39 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/*  40 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  41 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  42 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
/*  43 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CircularArcRenderStep + SolidColor SrcOver" },
/*  44 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  45 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  46 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor Src" },
/*  47 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor Clear" },
//--------
/*  48 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  49 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  50 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  51 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  52 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  53 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  54 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  55 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  56 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  57 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*  58 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  59 */ { 9, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
//--------
/*  60 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  61 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  62 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  63 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  64 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  65 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  66 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/*  67 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + SolidColor Src" },
/*  68 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + SolidColor Clear" },
/*  69 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  70 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  71 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  72 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  73 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  74 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/*  75 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/*  76 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/*  77 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/*  78 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/*  79 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  80 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[InverseCover] + (empty)" },
/*  81 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
/*  82 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CircularArcRenderStep + SolidColor SrcOver" },
/*  83 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/*  84 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/*  85 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + SolidColor Src" },
/*  86 */ { 9, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + SolidColor Clear" },
//--------
/*  87 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  88 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/*  89 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + SolidColor SrcOver" },
/*  90 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateStrokesRenderStep + SolidColor SrcOver" },
/*  91 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/*  92 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/*  93 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/*  94 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/*  95 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[RegularCover] + SolidColor SrcOver" },
/*  96 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[InverseCover] + SolidColor SrcOver" },
/*  97 */ { 9, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "CoverageMaskRenderStep + SolidColor SrcOver" },
//--------
/*  98 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "TessellateWedgesRenderStep[Winding] + (empty)" },
/*  99 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "TessellateWedgesRenderStep[EvenOdd] + (empty)" },
/* 100 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "TessellateWedgesRenderStep[Convex] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 101 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "TessellateStrokesRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 102 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "TessellateCurvesRenderStep[Winding] + (empty)" },
/* 103 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "TessellateCurvesRenderStep[EvenOdd] + (empty)" },
/* 104 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor SrcOver" },
/* 105 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Src" },
/* 106 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + SolidColor Clear" },
/* 107 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 108 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/* 109 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 119 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 111 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 112 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 113 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 114 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 115 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "MiddleOutFanRenderStep[Winding] + (empty)" },
/* 116 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "MiddleOutFanRenderStep[EvenOdd] + (empty)" },
/* 117 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[RegularCover] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 118 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/* 119 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Src" },
/* 120 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor Clear" },
/* 121 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 122 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/* 123 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 124 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 125 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 126 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 127 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 128 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 129 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 130 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 131 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[InverseCover] + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 132 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 133 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/* 134 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
/* 135 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor Src" },
/* 136 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor Clear" },
/* 137 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 138 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] Src" },
/* 139 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 140 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] Src" },
/* 141 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 142 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 143 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] Src" },
/* 144 */ { 9, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
//--------
/* 145 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 146 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 147 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 148 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 149 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 150 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 151 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 152 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 153 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 154 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/* 155 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 156 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 157 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 158 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 159 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 160 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 161 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 162 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 163 */ { 8, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/* 164 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] Src" },
/* 165 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformSRGB ] ] Src" },
/* 166 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 167 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 168 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] Src" },
/* 169 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformSRGB ] ] Src" },
/* 170 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 171 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformSRGB ] ] Src" },
/* 172 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ Image(0) ColorSpaceTransformSRGB ] ] Src" },
/* 173 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 174 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformSRGB ] ] Src" },
/* 175 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + LocalMatrix [ Compose [ CubicImage(0) ColorSpaceTransformPremul ] ] SrcOver" },
/* 176 */ { 8, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
//--------
/* 177 */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 178 */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 179 */ { 7, "RP((BGRA8+D16 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/* 180 */ { 7, "RP((BGRA8+D16 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + SolidColor SrcOver" },
/* 181 */ { 7, "RP((BGRA8+D16 x4->1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
//--------
/* 182 */ { 7, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 183 */ { 7, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/* 184 */ { 6, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 185 */ { 6, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 186 */ { 6, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
/* 187 */ { 6, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
//--------
/* 188 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 189 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 190 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/* 191 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver AnalyticClip" },
/* 192 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver" },
/* 193 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/* 194 */ { 6, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
//--------
/* 195 */ { 5, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + SolidColor SrcOver" },
/* 196 */ { 5, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] ] ] Src" },
/* 197 */ { 5, "RP((R8+D16 x1).a000) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver" },
//--------
/* 198 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 199 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 200 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/* 201 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverageMaskRenderStep + Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/* 202 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Mask] + LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/* 203 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "TessellateWedgesRenderStep[Convex] + (empty)" },
/* 204 */ { 5, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticBlurRenderStep + Compose [ SolidColor Bl\" },endCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
//--------
/* 205 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ YUVImage ColorSpaceTransformSRGB ] ] SrcOver AnalyticClip" },
/* 206 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] DstIn" },
/* 207 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + Compose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 208 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransformPremul ] ] ] Src" },
/* 209 */ { 5, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + SolidColor SrcOver AnalyticClip" },
    };

[[maybe_unused]] void find_duplicates(SkSpan<const ChromePipeline> cases) {
    for (size_t i = 0; i < std::size(cases); ++i) {
        for (size_t j = i+1; j < std::size(cases); ++j) {
            if (!strcmp(cases[i].fString, cases[j].fString)) {
                SkDebugf("Duplicate %zu && %zu\n", i, j);
            }
        }
    }
}

std::string rm_whitespace(const std::string& s) {
    auto start = s.find_first_not_of(' ');
    auto end = s.find_last_not_of(' ');
    return s.substr(start, (end - start) + 1);
}

// Precompile with the provided PrecompileSettings then verify that:
//   1) some case in 'kCases' is covered
//   2) more than 40% of the generated Pipelines are in kCases
void run_test(skgpu::graphite::PrecompileContext* precompileContext,
              skiatest::Reporter* reporter,
              const PrecompileSettings& settings,
              int precompileSettingsIndex,
              std::vector<bool>* casesThatAreMatched) {
    using namespace skgpu::graphite;

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext,
               settings.fPaintOptions,
               settings.fDrawTypeFlags,
               { &settings.fRenderPassProps, 1 });

    std::set<std::string> generatedLabels;

    {
        const RendererProvider* rendererProvider = precompileContext->priv().rendererProvider();
        const ShaderCodeDictionary* dict = precompileContext->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> generatedKeys;

        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &generatedKeys);

        for (const skgpu::UniqueKey& key : generatedKeys) {
            GraphicsPipelineDesc pipelineDesc;
            RenderPassDesc renderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(precompileContext, key, &pipelineDesc, &renderPassDesc);

            const RenderStep* renderStep = rendererProvider->lookup(pipelineDesc.renderStepID());
            std::string tmp = GetPipelineLabel(dict, renderPassDesc, renderStep,
                                               pipelineDesc.paintParamsID());
            generatedLabels.insert(rm_whitespace(tmp));
        }
    }

    std::vector<bool> localMatches;
    std::vector<size_t> matchesInCases;

    for (const std::string& g : generatedLabels) {
        bool didThisLabelMatch = false;
        for (size_t j = 0; j < std::size(kCases); ++j) {
            const char* testStr = kCases[j].fString;
            if (!strcmp(g.c_str(), testStr)) {

#if defined(SK_DEBUG)
                DrawTypeFlags expectedFlags = get_draw_type_flags(testStr);
                SkASSERT(expectedFlags & settings.fDrawTypeFlags);
                RenderPassProperties expectedRPP = get_render_pass_properties(testStr);
                if (strstr(testStr, "ColorSpaceTransformSRGB")) {
                    expectedRPP.fDstCS = SkColorSpace::MakeSRGB();
                }
                SkASSERT(expectedRPP == settings.fRenderPassProps);
#endif

                didThisLabelMatch = true;
                matchesInCases.push_back(j);
                (*casesThatAreMatched)[j] = true;
            }
        }

        localMatches.push_back(didThisLabelMatch);
    }

    REPORTER_ASSERT(reporter, matchesInCases.size() >= 1,   // This tests requirement 1, above
                    "%d: num matches: %zu", precompileSettingsIndex, matchesInCases.size());
    float utilization = ((float) matchesInCases.size())/generatedLabels.size();
    REPORTER_ASSERT(reporter, utilization >= 0.4f,         // This tests requirement 2, above
                    "%d: utilization: %f", precompileSettingsIndex, utilization);

#if defined(PRINT_COVERAGE)
    // This block will print out all the cases in 'kCases' that the given PrecompileSettings
    // covered.
    sort(matchesInCases.begin(), matchesInCases.end());
    SkDebugf("precompile case %d handles %zu/%zu cases (%.2f utilization): ",
             precompileSettingsIndex, matchesInCases.size(), generatedLabels.size(), utilization);
    for (size_t h : matchesInCases) {
        SkDebugf("%zu ", h);
    }
    SkDebugf("\n");
#endif

#if defined(PRINT_GENERATED_LABELS)
    // This block will print out all the labels from the given PrecompileSettings marked with
    // whether they were found in 'kCases'. This is useful for analyzing the set of Pipelines
    // generated by a single PrecompileSettings and is usually used along with 'kChosenCase'.
    SkASSERT(localMatches.size() == generatedLabels.size());

    int index = 0;
    for (const std::string& g : generatedLabels) {
        SkDebugf("%c %d: %s\n", localMatches[index] ? 'h' : ' ', index, g.c_str());
        ++index;
    }
#endif
}

[[maybe_unused]] bool skip(const char* str) {
    if (strstr(str, "AnalyticClip")) {  // we have to think about this a bit more
        return true;
    }
    if (strstr(str, "AnalyticBlurRenderStep")) { // currently internal only
        return true;
    }
    if (strstr(str, "KnownRuntimeEffect_1DBlur16")) {  // we have to revise how we do blurring
        return true;
    }
    if (strstr(str, "LinearGradient4")) {  // this seems too specialized
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

    std::vector<bool> casesThatAreMatched(std::size(kCases), false);

    static const size_t kChosenCase = -1;  // only test this entry in 'kPrecompileCases'
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        if (kChosenCase != -1 && kChosenCase != i) {
            continue;
        }

        run_test(precompileContext.get(), reporter,
                 kPrecompileCases[i], i, &casesThatAreMatched);
    }

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.
    int numCovered = 0, numNotCovered = 0, numIntentionallySkipped = 0;
    SkDebugf("not covered: ");
    for (size_t i = 0; i < std::size(kCases); ++i) {
        if (!casesThatAreMatched[i]) {
            if (skip(kCases[i].fString)) {
                ++numIntentionallySkipped;
            } else {
                SkDebugf("%zu, ", i);
                ++numNotCovered;
            }
        } else {
            ++numCovered;
        }
    }
    SkDebugf("\n");
    SkDebugf("covered %d notCovered %d skipped %d total %zu\n",
             numCovered, numNotCovered, numIntentionallySkipped,
             std::size(kCases));
#endif
}

#endif // SK_GRAPHITE
