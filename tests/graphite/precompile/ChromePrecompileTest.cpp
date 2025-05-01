/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "src/base/SkMathPriv.h"
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

using namespace skgpu::graphite;
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

namespace {

using ::skgpu::graphite::DepthStencilFlags;
using ::skgpu::graphite::DrawTypeFlags;
using ::skgpu::graphite::PaintOptions;
using ::skgpu::graphite::RenderPassProperties;

PaintOptions solid_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions linear_grad_sm_srcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient(GradientShaderFlags::kSmall) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions linear_grad_SRGB_sm_med_srcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient(
            GradientShaderFlags::kNoLarge,
            { SkGradientShader::Interpolation::InPremul::kNo,
              SkGradientShader::Interpolation::ColorSpace::kSRGB,
              SkGradientShader::Interpolation::HueMethod::kShorter }) });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setDither(true);

    return paintOptions;
}

PaintOptions image_premul_hw_and_clamp_srcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kClamp;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions image_premul_hw_only_srcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions xparent_paint_srcover() {
    PaintOptions paintOptions;

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions solid_clear_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kClear,
                                 SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions solid_src_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_premul_no_cubic_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kClamp;
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_premul_clamp_no_cubic_dstin() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kClamp;
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1}) });
    paintOptions.setBlendModes({ SkBlendMode::kDstIn });
    return paintOptions;
}

PaintOptions image_premul_hw_only_dstin() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kDstIn });
    return paintOptions;
}

PaintOptions yuv_image_srgb_no_cubic_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::YUVImage(YUVImageShaderFlags::kExcludeCubic,
                                                          { &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions yuv_image_srgb_srcover2() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::YUVImage(
                                                YUVImageShaderFlags::kNoCubicNoNonSwizzledHW,
                                                { &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_premul_no_cubic_src_srcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions image_srgb_no_cubic_src() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                           SkNamedGamut::kAdobeRGB) };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

[[maybe_unused]] PaintOptions blend_porter_duff_cf_srcover() {
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

// The same as kBGRA_1_D but w/ an Adobe RGB colorSpace
const RenderPassProperties kBGRA_1_D_Adobe { DepthStencilFlags::kDepth,
                                             kBGRA_8888_SkColorType,
                                             SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                                   SkNamedGamut::kAdobeRGB),
                                             /* fRequiresMSAA= */ false };

// The same as kBGRA_4_DS but w/ an SRGB colorSpace
const RenderPassProperties kBGRA_4_DS_SRGB { DepthStencilFlags::kDepthStencil,
                                             kBGRA_8888_SkColorType,
                                             SkColorSpace::MakeSRGB(),
                                             /* fRequiresMSAA= */ true };

// The same as kBGRA_4_DS but w/ an Adobe RGB colorSpace
const RenderPassProperties kBGRA_4_DS_Adobe { DepthStencilFlags::kDepthStencil,
                                              kBGRA_8888_SkColorType,
                                              SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                                    SkNamedGamut::kAdobeRGB),
                                              /* fRequiresMSAA= */ true };

constexpr DrawTypeFlags kRRectAndNonAARect =
        static_cast<DrawTypeFlags>(DrawTypeFlags::kAnalyticRRect | DrawTypeFlags::kNonAAFillRect);
constexpr DrawTypeFlags kQuadAndNonAARect =
        static_cast<DrawTypeFlags>(DrawTypeFlags::kPerEdgeAAQuad | DrawTypeFlags::kNonAAFillRect);

// These settings cover 94 of the 255 cases in 'kCases'.
// They create 118 Pipelines so only modestly over-generate.
const struct PrecompileSettings {
    PaintOptions fPaintOptions;
    DrawTypeFlags fDrawTypeFlags = DrawTypeFlags::kNone;
    RenderPassProperties fRenderPassProps;

    bool isSubsetOf(const PrecompileSettings& superSet) const {
        SkASSERT(SkPopCount(static_cast<uint32_t>(fDrawTypeFlags)) == 1);

        // 'superSet' may have a wider range of DrawTypeFlags
        return (fDrawTypeFlags & superSet.fDrawTypeFlags) &&
                fRenderPassProps == superSet.fRenderPassProps;
    }

} kPrecompileCases[] = {

//-----------------
/*  0 */ { solid_srcover(),                    DrawTypeFlags::kBitmapText_Mask,  kBGRA_1_D },
//-----------------
/*  1 */ { solid_srcover(),                    DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_D  },
//-----------------
/*  2 */ { solid_srcover(),                    DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_DS },
/*  3 */ { linear_grad_sm_srcover(),           DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_DS },
/*  4 */ { blend_porter_duff_cf_srcover(),     DrawTypeFlags::kBitmapText_Mask,  kBGRA_4_DS },

//-----------------
/*  5 */ { xparent_paint_srcover(),            DrawTypeFlags::kBitmapText_Color, kBGRA_1_D },
/*  6 */ { solid_srcover(),                    DrawTypeFlags::kBitmapText_Color, kBGRA_1_D_Adobe },
//-----------------
/*  7 */ { solid_srcover(),                    DrawTypeFlags::kBitmapText_Color, kBGRA_4_DS_Adobe },

//-----------------
/*  8 */ { solid_srcover(),                    kRRectAndNonAARect,               kR_1_D },

//-----------------
/*  9 */ { solid_src_srcover(),                DrawTypeFlags::kSimpleShape,      kBGRA_1_D },
/* 10 */ { image_premul_no_cubic_src_srcover(),kQuadAndNonAARect,                kBGRA_1_D },
/* 11 */ { image_premul_hw_and_clamp_srcover(),kQuadAndNonAARect,                kBGRA_1_D },
/* 12 */ { image_premul_clamp_no_cubic_dstin(),kQuadAndNonAARect,                kBGRA_1_D },
/* 13 */ { linear_grad_SRGB_sm_med_srcover(),  kRRectAndNonAARect,               kBGRA_1_D_Adobe },
/* 14 */ { image_srgb_no_cubic_src(),          kQuadAndNonAARect,                kBGRA_1_D_SRGB },
/* 15 */ { yuv_image_srgb_no_cubic_srcover(),  DrawTypeFlags::kSimpleShape,      kBGRA_1_D_SRGB },

//-----------------
/* 16 */ { solid_src_srcover(),                kRRectAndNonAARect,               kBGRA_4_D },

//-----------------
/* 17 */ { solid_srcover(),                    DrawTypeFlags::kNonSimpleShape,   kBGRA_4_DS },
/* 18 */ { solid_srcover(),                    DrawTypeFlags::kAnalyticRRect,    kBGRA_4_DS },
/* 19 */ { solid_clear_src_srcover(),          DrawTypeFlags::kNonAAFillRect,    kBGRA_4_DS },
/* 20 */ { image_premul_no_cubic_srcover(),    kQuadAndNonAARect,                kBGRA_4_DS },
/* 21 */ { image_premul_hw_only_dstin(),       DrawTypeFlags::kPerEdgeAAQuad,    kBGRA_4_DS },
/* 22 */ { image_premul_hw_only_srcover(),     DrawTypeFlags::kPerEdgeAAQuad,    kBGRA_4_DS },
/* 23 */ { linear_grad_SRGB_sm_med_srcover(),  kRRectAndNonAARect,               kBGRA_4_DS_Adobe },
/* 24 */ { yuv_image_srgb_srcover2(),          DrawTypeFlags::kSimpleShape,      kBGRA_4_DS_SRGB },
};

/*********** Here ends the part that can be pasted into Chrome's graphite_precompile.cc ***********/

#if defined(SK_DEBUG)
// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
RenderPassProperties get_render_pass_properties(const char* str) {
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
DrawTypeFlags get_draw_type_flags(const char* str) {
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

void deduce_settings_from_label(const char* testStr, PrecompileSettings* result) {
    result->fDrawTypeFlags = get_draw_type_flags(testStr);
    result->fRenderPassProps = get_render_pass_properties(testStr);
    if (strstr(testStr, "LinearGradient4 ColorSpaceTransformSRGB") ||
        strstr(testStr, "LinearGradient8 ColorSpaceTransformSRGB") ||
        strstr(testStr, "PrimitiveColor ColorSpaceTransformSRGB")) {
        result->fRenderPassProps.fDstCS = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                               SkNamedGamut::kAdobeRGB);
    } else if (strstr(testStr, "ColorSpaceTransformSRGB")) {
        result->fRenderPassProps.fDstCS = SkColorSpace::MakeSRGB();
    }
}

#endif // SK_DEBUG

struct ChromePipeline {
    int fNumHits;         // the number of uses in 9 of the 14 most visited web sites
    const char* fString;
};

//
// These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
// by collecting all the Pipelines from 9 of the top 14 visited sites according to Wikipedia
//
static const ChromePipeline kCases[] = {
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
/*   ? */ { 4, "RP((R8+D16 x1).a000) + "                             //-----------------------------
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] SrcOver" },
/*  60 */ { 4, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "SolidColor SrcOver" },
/*   X */ { 4, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] ] ] Src" },
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
/*   ? */ { 3, "RP((R8+D16 x1).a000) + "                             //-----------------------------
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ BlendCompose [ Compose [ Image(0) ColorSpaceTransform ] RGBPaintColor DstIn ] ] Src" },
/*   X */ { 3, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur16 [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] ] Src" },
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
/*   ? */ { 3, "RP((BGRA8+D16 x1).rgba) + "                          //-----------------------------
               "PerEdgeAAQuadRenderStep + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/* 117 */ { 3, "RP((BGRA8+D16 x1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*   ? */ { 3, "RP((BGRA8+D16 x1).rgba) + "                          //-----------------------------
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
/*   ? */ { 3, "RP((BGRA8+D16 x1).rgba) + "                          //-----------------------------
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
//--------
/*   ? */ { 2, "RP((R8+D16 x1).a000) + "                             //-----------------------------
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ BlendCompose [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] RGBPaintColor DstIn ] ] Src" },
/*   X */ { 2, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur4 [ LocalMatrix [ Compose [ ImageShaderClamp(0) ColorSpaceTransform ] ] ] Src" },
/*   X */ { 2, "RP((R8+D16 x1).a000) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "KnownRuntimeEffect_1DBlur4 [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransform ] ] ] Src" },
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
/*   ? */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba) + "                    //-----------------------------
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
/*   ? */ { 2, "RP((BGRA8+D16 x4->1).rgba) + "                       //-----------------------------
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
/*   ? */ { 2, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "       //-----------------------------
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
//$$$$$$ 156^
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
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ HWYUVImageNoSwizzle ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba) + "
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
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
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
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba) + "
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
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "CoverageMaskRenderStep + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformSRGB ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load) + "
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
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
               "PerEdgeAAQuadRenderStep + "
               "BlendCompose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x4->1).rgba w/ msaa load) + "
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
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
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
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "CoverBoundsRenderStep[NonAAFill] + "
               "Compose [ LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformPremul ] ] MatrixColorFilter ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
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
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Mask] + "
               "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "BitmapTextRenderStep[Color] + "
               "BlendCompose [ BlendCompose [ RGBPaintColor Compose [ PrimitiveColor ColorSpaceTransformPremul ] PorterDuffBlender ] AlphaOnlyPaintColor SrcIn ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "LocalMatrix [ Compose [ CoordNormalize [ HardwareImage(0) ] ColorSpaceTransformSRGB ] ] SrcOver" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver AnalyticClip" },
/*     */ { 1, "RP((BGRA8+D16 x1).rgba) + "
               "AnalyticRRectRenderStep + "
               "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformSRGB ] ] Dither ] SrcOver" },
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

[[maybe_unused]] bool skip(const char* str) {
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

// PipelineLabelInfo captures the information for a single Pipeline label. It stores which
// entry in 'kCases' it represents and which entry in 'kPrecompileCases' fulfilled it.
class PipelineLabelInfo {
public:
    PipelineLabelInfo(int casesIndex, int val = kUninit)
        : fCasesIndex(casesIndex)
        , fPrecompileCase(val) {}

    // Index of this Pipeline label in 'kCases'.
    const int fCasesIndex;

    static constexpr int kSkipped = -2;
    static constexpr int kUninit  = -1;
    // >= 0 -> covered by the 'fPrecompileCase' case in 'kPrecompileCases'
    int fPrecompileCase = kUninit;
};

class PipelineLabelInfoCollector {
public:
    PipelineLabelInfoCollector() {
        for (size_t i = 0; i < std::size(kCases); ++i) {
            const char* testStr = kCases[i].fString;

            if (skip(testStr)) {
                fMap.insert({ testStr, PipelineLabelInfo(i, PipelineLabelInfo::kSkipped) });
            } else {
                fMap.insert({ testStr, PipelineLabelInfo(i) });
            }
        }
    }

    int processLabel(const std::string& precompiledLabel, int precompileCase) {
        ++fNumLabelsProcessed;

        auto result = fMap.find(precompiledLabel.c_str());
        if (result == fMap.end()) {
            SkASSERT(fOverGenerated.find(precompiledLabel) == fOverGenerated.end());
            fOverGenerated.insert({ precompiledLabel, OverGenInfo(precompileCase) });
            return -1;
        }

        SkASSERT(result->second.fPrecompileCase == PipelineLabelInfo::kUninit);
        result->second.fPrecompileCase = precompileCase;
        return result->second.fCasesIndex;
    }

    void finalReport() {
        std::vector<int> skipped, missed;
        int numCovered = 0, numIntentionallySkipped = 0, numMissed = 0;

        for (const auto& iter : fMap) {
            if (iter.second.fPrecompileCase == PipelineLabelInfo::kSkipped) {
                ++numIntentionallySkipped;
                skipped.push_back(iter.second.fCasesIndex);
            } else if (iter.second.fPrecompileCase == PipelineLabelInfo::kUninit) {
                ++numMissed;
                missed.push_back(iter.second.fCasesIndex);
            } else {
                SkASSERT(iter.second.fPrecompileCase >= 0);
                ++numCovered;
            }
        }

        SkASSERT(numMissed == (int) missed.size());
        SkASSERT(numIntentionallySkipped == (int) skipped.size());

        SkDebugf("-----------------------\n");
        sort(missed.begin(), missed.end());
        SkDebugf("not covered: ");
        for (int i : missed) {
            SkDebugf("%d, ", i);
        }
        SkDebugf("\n");

        sort(skipped.begin(), skipped.end());
        SkDebugf("skipped: ");
        for (int i : skipped) {
            SkDebugf("%d, ", i);
        }
        SkDebugf("\n");

        SkASSERT(numCovered + static_cast<int>(fOverGenerated.size()) == fNumLabelsProcessed);

        SkDebugf("covered %d notCovered %d skipped %d total %zu\n",
                 numCovered,
                 numMissed,
                 numIntentionallySkipped,
                 fMap.size());
        SkDebugf("%d Pipelines were generated\n", fNumLabelsProcessed);
        SkDebugf("of that %zu Pipelines were over-generated:\n", fOverGenerated.size());
#if 0 // enable to print out a list of the over-generated Pipeline labels
        for (const auto& s : fOverGenerated) {
            SkDebugf("from %d: %s\n", s.second.fOriginatingSetting, s.first.c_str());
        }
#endif
    }

private:
    struct comparator {
        bool operator()(const char* a, const char* b) const {
            return strcmp(a, b) < 0;
        }
    };

    int fNumLabelsProcessed = 0;
    std::map<const char*, PipelineLabelInfo, comparator> fMap;

    struct OverGenInfo {
        OverGenInfo(int originatingSetting) : fOriginatingSetting(originatingSetting) {}

        int fOriginatingSetting;
    };

    std::map<std::string, OverGenInfo> fOverGenerated;
};

// Precompile with the provided PrecompileSettings then verify that:
//   1) some case in 'kCases' is covered
//   2) more than 40% of the generated Pipelines are in kCases
void run_test(skgpu::graphite::PrecompileContext* precompileContext,
              skiatest::Reporter* reporter,
              int precompileSettingsIndex,
              PipelineLabelInfoCollector* collector) {
    using namespace skgpu::graphite;

    const PrecompileSettings& settings = kPrecompileCases[precompileSettingsIndex];

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
        int matchInCases = collector->processLabel(g, precompileSettingsIndex);
        localMatches.push_back(matchInCases >= 0);

        if (matchInCases >= 0) {
            matchesInCases.push_back(matchInCases);

#if defined(SK_DEBUG)
            {
                PrecompileSettings expectedSettings;

                deduce_settings_from_label(kCases[matchInCases].fString, &expectedSettings);
                SkASSERT(expectedSettings.isSubsetOf(settings));
            }
#endif
        }
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

    PipelineLabelInfoCollector collector;

    static const size_t kChosenCase = -1;  // only test this entry in 'kPrecompileCases'
    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        if (kChosenCase != -1 && kChosenCase != i) {
            continue;
        }

        run_test(precompileContext.get(), reporter, i, &collector);
    }

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'kCases' that
    // were not covered by the PaintOptions.

    collector.finalReport();
#endif
}

#endif // SK_GRAPHITE
