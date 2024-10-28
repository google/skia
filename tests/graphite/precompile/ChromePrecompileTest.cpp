/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "tools/graphite/UniqueKeyUtils.h"

using namespace::skgpu::graphite;

namespace {

// "SolidColor SrcOver"
PaintOptions solid_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "SolidColor Src"
PaintOptions solid_src() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] SrcOver"
PaintOptions image_srcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] Src"
PaintOptions image_src() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image() });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

// "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] SrcOver"
PaintOptions lineargrad_srcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

// "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver"
PaintOptions lineargrad_srcover_dithered() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setDither(/* dither= */ true);
    return paintOptions;
}

// "Compose [ SolidColor Blend [ SolidColor Passthrough BlendModeBlender ] ] SrcOver"
[[maybe_unused]] PaintOptions blend_color_filter_srcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setColorFilters({ PrecompileColorFilters::Blend() });
    return paintOptions;
}

// "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba)"
// Single sampled BGRA w/ just depth
RenderPassProperties bgra_1_depth() {
    return { DepthStencilFlags::kDepth, kBGRA_8888_SkColorType, /* requiresMSAA= */ false };
}

// "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ just depth
RenderPassProperties bgra_4_depth() {
    return { DepthStencilFlags::kDepth, kBGRA_8888_SkColorType, /* requiresMSAA= */ true };
}

// "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba)"
// MSAA BGRA w/ depth and stencil
RenderPassProperties bgra_4_depth_stencil() {
    return { DepthStencilFlags::kDepthStencil, kBGRA_8888_SkColorType, /* requiresMSAA= */ true };
}

// Precompile with the provided paintOptions, drawType, and RenderPassSettings then verify that
// the expected string is in the generated set.
// Additionally, verify that overgeneration is within expected tolerances.
// If you add an additional RenderStep you may need to increase the tolerance values.
void run_test(Context* context,
              skiatest::Reporter* reporter,
              const char* expectedString, size_t caseID,
              const PaintOptions& paintOptions,
              DrawTypeFlags drawType,
              const RenderPassProperties& renderPassSettings,
              unsigned int allowedOvergeneration) {

    context->priv().globalCache()->resetGraphicsPipelines();

    Precompile(context, paintOptions, drawType, { &renderPassSettings, 1 });

    std::vector<std::string> generated;

    {
        const RendererProvider* rendererProvider = context->priv().rendererProvider();
        const ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> generatedKeys;

        UniqueKeyUtils::FetchUniqueKeys(context->priv().globalCache(), &generatedKeys);

        for (const skgpu::UniqueKey& key : generatedKeys) {
            GraphicsPipelineDesc pipelineDesc;
            RenderPassDesc renderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(context, key, &pipelineDesc, &renderPassDesc);

            const RenderStep* renderStep = rendererProvider->lookup(pipelineDesc.renderStepID());
            generated.push_back(GetPipelineLabel(dict, renderPassDesc, renderStep,
                                                 pipelineDesc.paintParamsID()));
        }
    }

    bool correctGenerationAmt = generated.size() == allowedOvergeneration;
    REPORTER_ASSERT(reporter, correctGenerationAmt,
                    "case %zu overgenerated - %zu > %d\n",
                    caseID, generated.size(), allowedOvergeneration);

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

    const Caps* caps = context->priv().caps();

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

    // In the following, here is the Dawn mapping from surface type to ID
    //    RGBA8Unorm = 18
    //    BGRA8Unorm = 23
    //    Depth16Unorm = 39
    //    Depth24PlusStencil8 = 41

    const char* kCases[] = {
        // Wikipedia 2018 - these are reordered from the spreadsheet
        /*  0 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateWedgesRenderStep[winding] + "
                 "(empty)",
        /*  1 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateWedgesRenderStep[evenodd] + "
                 "(empty)",
        /*  2 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "SolidColor SrcOver",
        /*  3 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "SolidColor Src",
        /*  4 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "PerEdgeAAQuadRenderStep + "
                 "LocalMatrix [ Compose [ Image(0) ColorSpaceTransform ] ] SrcOver",
        /*  5 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "PerEdgeAAQuadRenderStep + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] SrcOver",
        /*  6 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] SrcOver",
        /*  7 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "AnalyticRRectRenderStep + "
                 "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        /*  8 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
        /*  9 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "BitmapTextRenderStep[mask] + "
                 "LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] SrcOver",
        /* 10 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=41,s=4), samples: 4, swizzle: rgba) + "
                 "BitmapTextRenderStep[mask] + "
                 "SolidColor SrcOver",
        /* 11 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "AnalyticRRectRenderStep + "
                 "SolidColor SrcOver",
        /* 12 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "SolidColor SrcOver",
        /* 13 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "PerEdgeAAQuadRenderStep + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] Src",
        /* 14 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "LocalMatrix [ Compose [ HardwareImage(0) ColorSpaceTransform ] ] SrcOver",
        /* 15 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateWedgesRenderStep[convex] + "
                 "SolidColor SrcOver",
        /* 16 */ "RP(color: Dawn(f=23,s=4), resolve: Dawn(f=23,s=1), ds: Dawn(f=39,s=4), samples: 4, swizzle: rgba) + "
                 "TessellateStrokesRenderStep + "
                 "SolidColor SrcOver",
        /* 17 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "AnalyticBlurRenderStep + "
                 "Compose [ SolidColor Blend [ SolidColor Passthrough BlendModeBlender ] ] SrcOver",
        /* 18 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "SolidColor Src",
        /* 19 */ "RP(color: Dawn(f=23,s=1), resolve: {}, ds: Dawn(f=39,s=1), samples: 1, swizzle: rgba) + "
                 "CoverBoundsRenderStep[non-aa-fill] + "
                 "Compose [ LocalMatrix [ Compose [ LinearGradient4 ColorSpaceTransform ] ] Dither ] SrcOver",
    };

    for (size_t i = 0; i < std::size(kCases); ++i) {
        PaintOptions paintOptions;
        DrawTypeFlags drawTypeFlags = DrawTypeFlags::kSimpleShape;
        RenderPassProperties renderPassSettings;
        unsigned int allowedOvergeneration = 0;

        switch (i) {
            case 0:            [[fallthrough]];
            case 1:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 11;
                break;
            case 2:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 5;
                break;
            case 3: // only differs from 18 by MSAA and depth vs depth-stencil
                paintOptions = solid_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 5; // a lot for a rectangle clear - all RenderSteps
                break;
            case 4: // 4 is part of an AA image rect draw that can't use HW tiling
            case 5: // 5 & 6 together make up an AA image rect draw w/ a filled center
            case 6:
                paintOptions = image_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 30;
                break;
            case 7: // 7 & 8 are combined pair
            case 8:
                paintOptions = lineargrad_srcover_dithered();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 15; // 3x from gradient, 12x from RenderSteps
                break;
            case 9:
                paintOptions = lineargrad_srcover();
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 3; // from the 3 internal gradient alternatives
                break;
            case 10:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kBitmapText_Mask;
                renderPassSettings = bgra_4_depth_stencil();
                allowedOvergeneration = 1;
                break;
            case 11: // 11 & 12 are a pair - an RRect draw w/ a non-aa-fill center
            case 12:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_1_depth();
                allowedOvergeneration = 5;  // all from RenderSteps
                break;
            case 13:
                paintOptions = image_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_1_depth();
                // This is a lot for a kSrc image draw:
                allowedOvergeneration = 30; // 3x of this are the paint combos,
                                            // the rest are the RenderSteps!!
                break;
            case 14:
                paintOptions = image_srcover();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_1_depth();
                allowedOvergeneration = 30; // !!!! - a lot for just a non-aa image rect draw
                break;
            case 15:
            case 16:
                paintOptions = solid_srcover();
                drawTypeFlags = DrawTypeFlags::kNonSimpleShape;
                renderPassSettings = bgra_4_depth();
                allowedOvergeneration = 11;
                break;
            case 17:
                // After https://skia-review.googlesource.com/c/skia/+/887476 ([graphite] Split up
                // universal blend shader snippet) this case no longer exists/is reproducible.
                //
                //  paintOptions = blend_color_filter_srcover();
                //  drawTypeFlags = DrawTypeFlags::kSimpleShape;
                //  renderPassSettings = bgra_1_depth();
                //  allowedOvergeneration = 4;
                continue;
            case 18: // only differs from 3 by MSAA and depth vs depth-stencil
                paintOptions = solid_src();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_1_depth();
                allowedOvergeneration = 5; // a lot for a rectangle clear - all RenderSteps
                break;
            case 19:
                paintOptions = lineargrad_srcover_dithered();
                drawTypeFlags = DrawTypeFlags::kSimpleShape;
                renderPassSettings = bgra_1_depth();
                allowedOvergeneration = 15; // 3x from gradient, rest from RenderSteps
                break;
            default:
                continue;
        }

        if (renderPassSettings.fRequiresMSAA && caps->loadOpAffectsMSAAPipelines()) {
            allowedOvergeneration *= 2; // due to ExpandResolveTextureLoadOp
        }

        run_test(context, reporter,
                 kCases[i], i,
                 paintOptions, drawTypeFlags, renderPassSettings, allowedOvergeneration);
    }
}

#endif // SK_GRAPHITE
