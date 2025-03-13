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
#include "tools/graphite/UniqueKeyUtils.h"

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

// "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000)",
// Single sampled R w/ just depth
const RenderPassProperties kR_1_D { DepthStencilFlags::kDepth,
                                    kAlpha_8_SkColorType,
                                    /* fDstCS= */ nullptr,
                                    /* fRequiresMSAA= */ false };

// "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
// MSAA R w/ depth and stencil
const RenderPassProperties kR_4_DS { DepthStencilFlags::kDepthStencil,
                                     kAlpha_8_SkColorType,
                                     /* fDstCS= */ nullptr,
                                     /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba)"
// Single sampled BGRA w/ just depth
const RenderPassProperties kBGRA_1_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ false };

// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ just depth
const RenderPassProperties kBGRA_4_D { DepthStencilFlags::kDepth,
                                       kBGRA_8888_SkColorType,
                                       /* fDstCS= */ nullptr,
                                       /* fRequiresMSAA= */ true };

// "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)"
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

const struct PrecompileSettings {
    PaintOptions fPaintOptions;
    DrawTypeFlags fDrawTypeFlags = DrawTypeFlags::kNone;
    RenderPassProperties fRenderPassProps;
} kPrecompileCases[] = {
    { blend_porter_duff_cf_srcover(), DrawTypeFlags::kNone,           kBGRA_1_D }, // unused
    { blend_porter_duff_cf_srcover(), DrawTypeFlags::kNonSimpleShape, kBGRA_1_D }, // 13
    { solid_srcover(), DrawTypeFlags::kNonSimpleShape,  kR_4_DS },    // 2,3
    { solid_srcover(), DrawTypeFlags::kBitmapText_Mask, kBGRA_1_D },  // 7
    { solid_srcover(), DrawTypeFlags::kBitmapText_Mask, kBGRA_4_DS }, // 24
    { solid_srcover(), DrawTypeFlags::kNonSimpleShape,  kBGRA_4_D },  // 21,22
    { solid_srcover(), DrawTypeFlags::kNonSimpleShape,  kBGRA_4_DS }, // 28,34,35,57,58,61,62,65
    { solid_srcover(), DrawTypeFlags::kCircularArc,     kBGRA_4_DS }, //26
    { solid_clear_src_srcover(),  DrawTypeFlags::kSimpleShape, kBGRA_1_D },      // 6,8,9,10,15
    { solid_clear_src_srcover(),  DrawTypeFlags::kSimpleShape, kBGRA_4_DS },     // 23,30,31,53
    { image_premul_src_srcover(), DrawTypeFlags::kSimpleShape, kBGRA_1_D },      // 11,16,17,19,38
    { image_premul_srcover(),     DrawTypeFlags::kSimpleShape, kBGRA_4_DS },     // 32,33,60
    { image_srgb_src(),           DrawTypeFlags::kSimpleShape, kBGRA_1_D_SRGB }, // 18
};

/*********** Here ends the part that can be pasted into Chrome's graphite_precompile.cc ***********/

// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
RenderPassProperties get_render_pass_properties(const char* str) {
    static const struct {
        const char* fStr;
        RenderPassProperties fRenderPassProperties;
    } kRenderPassPropertiesMapping[] = {
        { "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000)",
          kR_1_D },
        { "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000)",
          kR_4_DS },
        { "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba)",
          kBGRA_1_D },
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba)",
          kBGRA_4_D },
        { "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba)",
           kBGRA_4_DS },
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

        { "AnalyticRRectRenderStep",                     DrawTypeFlags::kSimpleShape  },
        { "CoverBoundsRenderStep[NonAAFill]",            DrawTypeFlags::kSimpleShape  },
        { "PerEdgeAAQuadRenderStep",                     DrawTypeFlags::kSimpleShape  },

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


// Precompile with the provided paintOptions, drawType, and RenderPassSettings then verify that
// the expected string is in the generated set.
// Additionally, verify that overgeneration is within expected tolerances.
// If you add an additional RenderStep you may need to increase the tolerance values.
void run_test(skgpu::graphite::PrecompileContext* precompileContext,
              skiatest::Reporter* reporter,
              SkSpan<const char*> cases,
              size_t caseID,
              const PrecompileSettings& settings,
              unsigned int expectedNumPipelines) {
    using namespace skgpu::graphite;

    const char* expectedString = cases[caseID];

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext,
               settings.fPaintOptions,
               settings.fDrawTypeFlags,
               { &settings.fRenderPassProps, 1 });

    std::vector<std::string> generated;

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
            generated.push_back(GetPipelineLabel(dict, renderPassDesc, renderStep,
                                                 pipelineDesc.paintParamsID()));
        }
    }

    bool correctGenerationAmt = generated.size() == expectedNumPipelines;
    REPORTER_ASSERT(reporter, correctGenerationAmt,
                    "case %zu generated unexpected amount - a: %zu != e: %d\n",
                    caseID, generated.size(), expectedNumPipelines);

    const size_t len = strlen(expectedString);

    bool foundIt = false;
    for (size_t i = 0; i < generated.size(); ++i) {
        // The generated strings have trailing whitespace
        if (!strncmp(expectedString, generated[i].c_str(), len)) {
            foundIt = true;
            break;
        }
    }

    REPORTER_ASSERT(reporter, foundIt);

#ifdef SK_DEBUG
    if (foundIt && correctGenerationAmt) {
        return;
    }

    SkDebugf("Expected string:\n%s\n%s in %zu strings:\n",
             expectedString,
             foundIt ? "found" : "NOT found",
             generated.size());

    for (size_t i = 0; i < generated.size(); ++i) {
        SkDebugf("%zu: %s\n", i, generated[i].c_str());
    }
#endif
}

// The pipeline strings were created using the Dawn Metal backend so that is the only viable
// comparison
bool is_dawn_metal_context_type(skgpu::ContextType type) {
    return type == skgpu::ContextType::kDawn_Metal;
}

} // anonymous namespace


DEF_GRAPHITE_TEST_FOR_CONTEXTS(ChromePrecompileTest, is_dawn_metal_context_type,
                               reporter, context, /* testContext */, CtsEnforcement::kNever) {
    using namespace skgpu::graphite;

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kBGRA_8888_SkColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 skgpu::Protected::kNo,
                                                                 skgpu::Renderable::kYes);

    TextureInfo msaaTex = caps->getDefaultMSAATextureInfo(textureInfo, Discardable::kYes);

    if (msaaTex.numSamples() <= 1) {
        // The following pipelines rely on having MSAA
        return;
    }

#ifdef SK_ENABLE_VELLO_SHADERS
    if (caps->computeSupport()) {
        // The following pipelines rely on not utilizing Vello
        return;
    }
#endif

    //
    // These Pipelines are candidates for inclusion in Chrome's precompile. They were generated
    // by collecting all the Pipelines from the following stories:
    //
    //    animometer_webgl_attrib_arrays, balls_javascript_canvas, canvas_05000_pixels_per_second,
    //    chip_tune, css_value_type_shadow, fill_shapes, ie_chalkboard, main_30fps_impl_60fps,
    //    new_tilings, transform_transitions_js_block, web_animations_staggered_infinite_iterations,
    //    wikipedia_2018
    //
    const char* kCases[] = {
          //---- Single-sample - R8Unorm/Depth16Unorm --> kR_1_D
/* 0 */  "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "KnownRuntimeEffect_1DBlur12 [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",

/* 1 */  "RP(color: Dawn(f=R8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: a000) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "KnownRuntimeEffect_1DBlur8  [ LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] ] Src",

          //---- MSAA4 - R8Unorm/Depth24PlusStencil8 --> kR_4_DS
/* 2 */  "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
         "TessellateWedgesRenderStep[EvenOdd] + "
         "(empty)",

/* 3 */  "RP(color: Dawn(f=R8,s=4), resolve: Dawn(f=R8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: a000) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "SolidColor SrcOver",

         //---- Single-sample - BGRA8Unorm/Depth16Unorm -> kBGRA_1_D
/* 4 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticBlurRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",

          // For now, we're going to pass on the AnalyticClip Pipelines
/* 5 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticBlurRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver AnalyticClip",

/* 6 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 7 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

/* 8 */  "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 9 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 10 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Clear",

/* 11 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",

          // For now, we're going to pass on the AnalyticClip Pipelines
/* 12 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",

/* 13 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverageMaskRenderStep + "
         "Compose [ SolidColor BlendCompose [ SolidColor Passthrough PorterDuffBlender ] ] SrcOver",

         // This is the only AlphaOnlyPaintColor case so, we're going to pass for now
/* 14 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",

/* 15 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "SolidColor SrcOver",

/* 16 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] Src",

/* 17 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 18 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src",

/* 19 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth16Unorm -> bgra_4_D
         // For now, we're going to pass on the AnalyticClip Pipelines
/* 20 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver AnalyticClip",

/* 21 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "TessellateStrokesRenderStep + "
         "SolidColor SrcOver",

/* 22 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "SolidColor SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth24PlusStencil8 --> kBGRA_4_DS
/* 23 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 24 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

         // This seems like a quirk of our test set. Is linear gradient text that common? Pass for now.
/* 25 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] SrcOver",

/* 26 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CircularArcRenderStep + "
         "SolidColor SrcOver",

/* 27 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[InverseCover] + "
         "(empty)",

/* 28 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "SolidColor SrcOver",

/* 29 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "(empty)",

/* 30 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 31 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Clear",

/* 32 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 33 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 34 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Winding] + "
         "(empty)",

/* 35 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[EvenOdd] + "
         "(empty)",

/* 36 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "SolidColor SrcOver",

//----------------------------
// These are leftover Pipelines that were generated by the stories but are pretty infrequently used.
// Some of them get picked up in the preceding cases
          //---- Single-sample - BGRA8Unorm/Depth16Unorm -> kBGRA_1_D
/* 37 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 38 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 39 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 40 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] SrcOver",

/* 41 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformSRGB ] ] Src",

/* 42 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

/* 43 */ "RP(color: Dawn(f=BGRA8,s=1), resolve: {}, ds: Dawn(f=D16,s=1), samples: 1, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth16Unorm -> kBGRA_4_D
/* 44 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "SolidColor SrcOver",

/* 45 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "BitmapTextRenderStep[Mask] + "
         "SolidColor SrcOver",

/* 46 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver",

/* 47 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 48 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 49 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

/* 50 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 51 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D16,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ HWYUVImage ColorSpaceTransformSRGB ] ] SrcOver",

         //---- MSAA4 - BGRA8Unorm/Depth24PlusStencil8 --> kBGRA_4_DS
/* 52 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "AnalyticRRectRenderStep + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 53 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor Src",

/* 54 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

         // For now, we're going to pass on the AnalyticClip Pipelines
/* 55 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[NonAAFill] + "
         "SolidColor SrcOver AnalyticClip",

/* 56 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "CoverBoundsRenderStep[RegularCover] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 57 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "MiddleOutFanRenderStep[Winding] + "
         "(empty)",

/* 58 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "MiddleOutFanRenderStep[EvenOdd] + "
         "(empty)",

/* 59 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "BlendCompose [ LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransformPremul ] ] AlphaOnlyPaintColor SrcIn ] SrcOver",

/* 60 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "PerEdgeAAQuadRenderStep + "
         "LocalMatrix [ Compose [ Image(0) ColorSpaceTransformPremul ] ] SrcOver",

/* 61 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateCurvesRenderStep[Winding] + "
         "(empty)",

/* 62 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateCurvesRenderStep[EvenOdd] + "
         "(empty)",

/* 63 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradientBuffer ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 64 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateWedgesRenderStep[Convex] + "
         "Compose [ LocalMatrix [ Compose [ LinearGradient8 ColorSpaceTransformPremul ] ] Dither ] SrcOver",

/* 65 */ "RP(color: Dawn(f=BGRA8,s=4), resolve: Dawn(f=BGRA8,s=1), ds: Dawn(f=D24_S8,s=4), samples: 4, swizzle: rgba) + "
         "TessellateStrokesRenderStep + "
         "SolidColor SrcOver",
    };

    for (size_t i = 0; i < std::size(kCases); ++i) {
        PrecompileSettings settings;

        // TODO(robertphillips): splitting kCases[i] into substrings (based on a " + " separator)
        // before passing to the helpers would make this prettier
        RenderPassProperties expectedRenderPassSettings = get_render_pass_properties(kCases[i]);
        unsigned int expectedNumPipelines = 0;

        switch (i) {
#if 0
            // Punting on the blur cases for now since they horribly over-generate
            case 0:             [[fallthrough]];
            case 1:
                renderPassSettings = kR_1_D;
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                paintOptions = blur_image();
                expectedNumPipelines = 11;
                break;
#endif

            // 2 and 3 form a single-draw pair and are, thus, addressed by the same settings
            case 2: [[fallthrough]];   // TessellateWedgesRenderStep[EvenOdd]
            case 3:                    // kSrcOver - CoverBoundsRenderStep[RegularCover]
                // solid_srcover, kNonSimpleShape, kR_4_DS
                settings = kPrecompileCases[2];
                expectedNumPipelines = 11;   // This is pretty bad for just 2 Pipelines
                break;

            // 4 and 13 share the same paintOptions
            case 4:  // AnalyticBlurRenderStep
#if 0
                // We need to handle AnalyticBlurs differently (b/403264070)
                // blend_porter_duff_color_filter_srcover(), kAnalyticBlur, kBGRA_1_D
                settings = kPrecompileCases[0];
                expectedNumPipelines = 1;
                break;
#else
                continue;
#endif

            case 13: // CoverageMaskRenderStep
                // this case could be greatly reduced if CoverageMaskRenderStep were split out
                // blend_porter_duff_color_filter_srcover(), kNonSimpleShape, kBGRA_1_D
                settings = kPrecompileCases[1];
                expectedNumPipelines = 11; // This is pretty bad for just 1 Pipeline
                break;

            // This is the only AlphaOnlyPaintColor case so, we're going to pass for now
            case 14:
                continue;

            // For now, we're passing on the AnalyticClip Pipelines
            case 5:  // same as 4 but with an AnalyticClip
            case 12:
            case 20:
            case 55:
                continue;

            // The two text pipelines (7 and 24) just differ in render pass settings (one MSAA, the
            // other not)
            case 7:
                // solid_srcover(), kBitmapText_Mask, kBGRA_1_D
                settings = kPrecompileCases[3];
                expectedNumPipelines = 1;
                break;
            case 24:
                // solid_srcover(), kBitmapText_Mask, kBGRA_4_DS
                settings = kPrecompileCases[4];
                expectedNumPipelines = 1;
                break;

            // 21 & 22 form a pair (since they both need kNonSimpleShape)
            case 21: // kSrcOver - TessellateStrokesRenderStep
            case 22: // kSrcOver - TessellateWedgesRenderStep[Convex]
                // solid_srcover(), kNonSimpleShape,  kBGRA_4_D
                settings = kPrecompileCases[5];
                expectedNumPipelines = 11;  // very bad for 2 pipelines
                break;

            // Skipping linear gradient text draw. It doesn't seem representative.
            case 25:
                continue;

            // TODO(robertphillips): these two cases aren't being generated!
            case 27:  // CoverBoundsRenderStep[InverseCover]
            case 29:  // CoverBoundsRenderStep[RegularCover]
                continue; // they should normally just fall through

            // the next 9 form a block (since they all need kNonSimpleShape)
            case 28: // kSrcOver - CoverBoundsRenderStep[RegularCover]
            case 34: // TessellateWedgesRenderStep[Winding]
            case 35: // TessellateWedgesRenderStep[EvenOdd]
            case 36: // kSrcOver - TessellateWedgesRenderStep[Convex]
            // related utility Pipelines
            case 57: // MiddleOutFanRenderStep[Winding]
            case 58: // MiddleOutFanRenderStep[EvenOdd]
            case 61: // TessellateCurvesRenderStep[Winding]
            case 62: // TessellateCurvesRenderStep[EvenOdd]
            case 65: // kSrcOver - TessellateStrokesRenderStep
                // solid_srcover(), kNonSimpleShape,  kBGRA_4_DS
                settings = kPrecompileCases[6];
                expectedNumPipelines = 11;  // not so bad for 9 pipelines
                break;

            // These are the same except for the blend mode
            // kSrcOver seems fine (since it gets 3 pipelines). kSrc and kClear seem like they
            // should be narrowed (since they're over-generating a lot)
            case 6:  // kSrcOver - AnalyticRRectRenderStep
            case 8:  // kSrcOver - CoverBoundsRenderStep[NonAAFill]
            case 9:  // kSrc     - CoverBoundsRenderStep[NonAAFill]
            case 10: // kClear   - CoverBoundsRenderStep[NonAAFill]
            case 15: // kSrcOver - PerEdgeAAQuadRenderStep
                // solid_clear_src_srcover(), kSimpleShape, kBGRA_1_D
                settings = kPrecompileCases[8];
                // This is 3 each for kClear, kSrc and kSrcOver:
                //     AnalyticRRectRenderStep
                //     CoverBoundsRenderStep[NonAAFill]
                //     PerEdgeAAQuadRenderStep
                expectedNumPipelines = 9; // This is pretty bad for 5 pipelines
                break;

            case 26: // kSrcOver - CircularArcRenderStep
                // solid_srcover(), kCircularArc, kBGRA_4_DS
                settings = kPrecompileCases[7];
                expectedNumPipelines = 1;
                break;

            case 23: // kSrcOver - AnalyticRRectRenderStep
            case 30: // kSrcOver - CoverBoundsRenderStep[NonAAFill]
            case 31: // kClear   - CoverBoundsRenderStep[NonAAFill]
            case 53: // kSrc     - CoverBoundsRenderStep[NonAAFill]
                //  solid_clear_src_srcover(), kSimpleShape, kBGRA_4_DS
                settings = kPrecompileCases[9];
                // This is 3 each for kClear, kSrc and kSrcOver:
                //     AnalyticRRectRenderStep
                //     CoverBoundsRenderStep[NonAAFill]
                //     PerEdgeAAQuadRenderStep
                expectedNumPipelines = 9; // This is pretty bad for 4 pipelines
                break;

            // For all the image paintOptions we could add the option to exclude cubics to
            // the public API
            case 11: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrc
            case 16: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrc
            case 17: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrcOver
            case 19: // PerEdgeAAQuadRenderStep + Image(0) + kSrcOver
            case 38: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrcOver
                // image_premul_src_srcover(), kSimpleShape, kBGRA_1_D
                settings = kPrecompileCases[10];
                expectedNumPipelines = 24; // a bad deal for 5 pipelines
                break;

            // same as except 16 except it has ColorSpaceTransformSRGB
            case 18: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrc
                // image_srgb_src(), kSimpleShape, kBGRA_1_D_SRGB
                settings = kPrecompileCases[12];
                expectedRenderPassSettings.fDstCS = SkColorSpace::MakeSRGB();
                expectedNumPipelines = 12;  // a bad deal for 1 pipeline
                break;

            case 32: // CoverBoundsRenderStep[NonAAFill] + HardwareImage(0) + kSrcOver
            case 33: // PerEdgeAAQuadRenderStep + HardwareImage(0) + kSrcOver
            case 60: // PerEdgeAAQuadRenderStep + Image(0) + kSrcOver
                // image_premul_srcover(), kSimpleShape, kBGRA_4_DS
                settings = kPrecompileCases[11];
                expectedNumPipelines = 12; // a bad deal for 3 pipelines
                break;
            default:
                continue;
        }

        SkAssertResult(settings.fRenderPassProps == expectedRenderPassSettings);
        DrawTypeFlags expectedDrawTypeFlags = get_draw_type_flags(kCases[i]);
        SkAssertResult(settings.fDrawTypeFlags == expectedDrawTypeFlags);

        if (settings.fRenderPassProps.fRequiresMSAA && caps->loadOpAffectsMSAAPipelines()) {
            expectedNumPipelines *= 2; // due to wgpu::LoadOp::ExpandResolveTexture
        }

        run_test(precompileContext.get(), reporter,
                 { kCases, std::size(kCases) }, i,
                 settings, expectedNumPipelines);
    }
}

#endif // SK_GRAPHITE
