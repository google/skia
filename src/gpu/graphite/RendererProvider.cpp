/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RendererProvider.h"

#include "include/core/SkPathTypes.h"
#include "include/core/SkVertices.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/InternalDrawTypeFlags.h"
#include "src/gpu/graphite/render/AnalyticBlurRenderStep.h"
#include "src/gpu/graphite/render/AnalyticRRectRenderStep.h"
#include "src/gpu/graphite/render/BitmapTextRenderStep.h"
#include "src/gpu/graphite/render/CircularArcRenderStep.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/CoverBoundsRenderStep.h"
#include "src/gpu/graphite/render/CoverageMaskRenderStep.h"
#include "src/gpu/graphite/render/MiddleOutFanRenderStep.h"
#include "src/gpu/graphite/render/PerEdgeAAQuadRenderStep.h"
#include "src/gpu/graphite/render/SDFTextLCDRenderStep.h"
#include "src/gpu/graphite/render/SDFTextRenderStep.h"
#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"
#include "src/gpu/graphite/render/TessellateStrokesRenderStep.h"
#include "src/gpu/graphite/render/TessellateWedgesRenderStep.h"
#include "src/gpu/graphite/render/VerticesRenderStep.h"
#include "src/sksl/SkSLUtil.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/VelloRenderer.h"
#endif

namespace skgpu::graphite {

bool RendererProvider::IsVelloRendererSupported(const Caps* caps) {
#ifdef SK_ENABLE_VELLO_SHADERS
    return caps->computeSupport();
#else
    return false;
#endif
}

// The destructor is intentionally defined here and not in the header file to allow forward
// declared types (such as `VelloRenderer`) to be defined as a `std::unique_ptr` parameter type
// in members.
RendererProvider::~RendererProvider() = default;

RendererProvider::RendererProvider(const Caps* caps, StaticBufferManager* bufferManager) {
    // This constructor requires all Renderers be densely packed so that it can simply iterate over
    // the fields directly and fill 'fRenderers' with every one that was initialized with a
    // non-empty renderer. While this is a little magical, it simplifies the rest of the logic
    // and can be enforced statically.
    static constexpr size_t kRendererSize = offsetof(RendererProvider, fRenderers) -
                                            offsetof(RendererProvider, fStencilTessellatedCurves);
    static_assert(kRendererSize % sizeof(Renderer) == 0, "Renderer declarations are not dense");

    const bool infinitySupport = caps->shaderCaps()->fInfinitySupport;

    // Single-step renderers don't share RenderSteps
    auto makeFromStep = [&](std::unique_ptr<RenderStep> singleStep, DrawTypeFlags drawTypes) {
        std::string name = "SingleStep[";
        name += singleStep->name();
        name += "]";
        return Renderer(name, drawTypes, this->assumeOwnership(std::move(singleStep)));
    };

    fConvexTessellatedWedges =
            makeFromStep(std::make_unique<TessellateWedgesRenderStep>(
                                 RenderStep::RenderStepID::kTessellateWedges_Convex,
                                 infinitySupport, kDirectDepthGreaterPass, bufferManager),
                         DrawTypeFlags::kNonSimpleShape);
    fTessellatedStrokes = makeFromStep(
            std::make_unique<TessellateStrokesRenderStep>(infinitySupport),
            DrawTypeFlags::kNonSimpleShape);
    fCoverageMask = makeFromStep(
            std::make_unique<CoverageMaskRenderStep>(),
            static_cast<DrawTypeFlags>(static_cast<int>(DrawTypeFlags::kNonSimpleShape) |
                                       static_cast<int>(InternalDrawTypeFlags::kCoverageMask)));

    static constexpr struct {
        skgpu::MaskFormat fFormat;
        DrawTypeFlags     fDrawType;
    } kBitmapTextVariants [] = {
        // We are using 565 here to represent LCD text, regardless of texture format
        { skgpu::MaskFormat::kA8,   DrawTypeFlags::kBitmapText_Mask  },
        { skgpu::MaskFormat::kA565, DrawTypeFlags::kBitmapText_LCD   },
        { skgpu::MaskFormat::kARGB, DrawTypeFlags::kBitmapText_Color }
    };

    for (auto textVariant : kBitmapTextVariants) {
        fBitmapText[int(textVariant.fFormat)] =
                makeFromStep(std::make_unique<BitmapTextRenderStep>(textVariant.fFormat),
                             textVariant.fDrawType);
    }
    for (bool lcd : {false, true}) {
        fSDFText[lcd] = lcd ? makeFromStep(std::make_unique<SDFTextLCDRenderStep>(),
                                           DrawTypeFlags::kSDFText_LCD)
                            : makeFromStep(std::make_unique<SDFTextRenderStep>(),
                                           DrawTypeFlags::kSDFText);
    }
    fAnalyticRRect = makeFromStep(
            std::make_unique<AnalyticRRectRenderStep>(bufferManager),
            static_cast<DrawTypeFlags>(static_cast<int>(DrawTypeFlags::kSimpleShape) |
                                       static_cast<int>(InternalDrawTypeFlags::kAnalyticRRect)));
    fPerEdgeAAQuad = makeFromStep(std::make_unique<PerEdgeAAQuadRenderStep>(bufferManager),
                                  DrawTypeFlags::kSimpleShape);
    fNonAABoundsFill = makeFromStep(std::make_unique<CoverBoundsRenderStep>(
                                            RenderStep::RenderStepID::kCoverBounds_NonAAFill,
                                            kDirectDepthGreaterPass),
                                    DrawTypeFlags::kSimpleShape);
    fCircularArc = makeFromStep(std::make_unique<CircularArcRenderStep>(bufferManager),
                                DrawTypeFlags::kSimpleShape);
    fAnalyticBlur = makeFromStep(std::make_unique<AnalyticBlurRenderStep>(),
                                 DrawTypeFlags::kSimpleShape);

    // vertices
    for (PrimitiveType primType : {PrimitiveType::kTriangles, PrimitiveType::kTriangleStrip}) {
        for (bool color : {false, true}) {
            for (bool texCoords : {false, true}) {
                int index = 4*(primType == PrimitiveType::kTriangleStrip) + 2*color + texCoords;
                fVertices[index] = makeFromStep(
                        std::make_unique<VerticesRenderStep>(primType, color, texCoords),
                        DrawTypeFlags::kDrawVertices);
            }
        }
    }

    // The tessellating path renderers that use stencil can share the cover steps.
    auto coverFill = std::make_unique<CoverBoundsRenderStep>(
            RenderStep::RenderStepID::kCoverBounds_RegularCover, kRegularCoverPass);
    auto coverInverse = std::make_unique<CoverBoundsRenderStep>(
            RenderStep::RenderStepID::kCoverBounds_InverseCover, kInverseCoverPass);

    for (bool evenOdd : {false, true}) {
        // These steps can be shared by regular and inverse fills
        auto stencilFan = std::make_unique<MiddleOutFanRenderStep>(evenOdd);
        auto stencilCurve = std::make_unique<TessellateCurvesRenderStep>(
                evenOdd, infinitySupport, bufferManager);
        auto stencilWedge =
                evenOdd ? std::make_unique<TessellateWedgesRenderStep>(
                                RenderStep::RenderStepID::kTessellateWedges_EvenOdd,
                                infinitySupport, kEvenOddStencilPass, bufferManager)
                        : std::make_unique<TessellateWedgesRenderStep>(
                                RenderStep::RenderStepID::kTessellateWedges_Winding,
                                infinitySupport, kWindingStencilPass, bufferManager);

        for (bool inverse : {false, true}) {
            static const char* kTessVariants[4] =
                    {"[winding]", "[evenodd]", "[inverse-winding]", "[inverse-evenodd]"};

            int index = 2*inverse + evenOdd; // matches SkPathFillType
            std::string variant = kTessVariants[index];

            const RenderStep* coverStep = inverse ? coverInverse.get() : coverFill.get();
            fStencilTessellatedCurves[index] = Renderer("StencilTessellatedCurvesAndTris" + variant,
                                                        DrawTypeFlags::kNonSimpleShape,
                                                        stencilFan.get(),
                                                        stencilCurve.get(),
                                                        coverStep);

            fStencilTessellatedWedges[index] = Renderer("StencilTessellatedWedges" + variant,
                                                        DrawTypeFlags::kNonSimpleShape,
                                                        stencilWedge.get(),
                                                        coverStep);
        }

        this->assumeOwnership(std::move(stencilFan));
        this->assumeOwnership(std::move(stencilCurve));
        this->assumeOwnership(std::move(stencilWedge));
    }

    this->assumeOwnership(std::move(coverInverse));
    this->assumeOwnership(std::move(coverFill));

    // Fill out 'fRenderers' by iterating the "span" from fStencilTessellatedCurves to fRenderers
    // and checking if they've been skipped or not.
    SkSpan<Renderer> allRenderers = {fStencilTessellatedCurves, kRendererSize / sizeof(Renderer)};
    for (const Renderer& r : allRenderers) {
        if (r.numRenderSteps() > 0) {
            fRenderers.push_back(&r);
        }
    }

#ifdef SK_ENABLE_VELLO_SHADERS
    fVelloRenderer = std::make_unique<VelloRenderer>(caps);
#endif
}

} // namespace skgpu::graphite
