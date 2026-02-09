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
#include "src/gpu/graphite/UniformManager.h"
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

bool RendererProvider::IsSupported(PathRendererStrategy strategy, const Caps* caps) {
    switch (strategy) {
        case PathRendererStrategy::kCPUSparseStripsMSAA8:
            return true;
        case PathRendererStrategy::kTessellationAndSmallAtlas:
            if (caps->minPathSizeForMSAA() <= 0) {
                return false; // Disabled explicitly
            }
            [[fallthrough]]; // Must support kTessellation too
        case PathRendererStrategy::kTessellation:
            // This strategy requires MSAA, which will use a supported MSAA count returned by
            // Caps::getDefaultMSAASampleCount(target). When avoidMSAA() returns false, this should
            // always be at least 4x on Graphite's supported devices.
            return !caps->avoidMSAA();

        case PathRendererStrategy::kRasterAtlas:
            // The raster path atlas is currently always supported
            return true;

        case PathRendererStrategy::kComputeAnalyticAA: [[fallthrough]];
        case PathRendererStrategy::kComputeMSAA16:
        case PathRendererStrategy::kComputeMSAA8:
            // The Vello compute strategies are supported if included in the build and has compute.
#if defined(SK_ENABLE_VELLO_SHADERS)
            return caps->computeSupport();
#else
            return false;
#endif
    }

    SkUNREACHABLE;
}

// The destructor is intentionally defined here and not in the header file to allow forward
// declared types (such as `VelloRenderer`) to be defined as a `std::unique_ptr` parameter type
// in members.
RendererProvider::~RendererProvider() = default;

RendererProvider::RendererProvider(const Caps* caps, StaticBufferManager* bufferManager) {
    // Determine path rendering strategy
#if defined(GPU_TEST_UTILS)
    if (caps->requestedPathRendererStrategy().has_value() &&
        IsSupported(*caps->requestedPathRendererStrategy(), caps)) {
        // Use the explicitly overridden strategy
        fStrategy = *caps->requestedPathRendererStrategy();
    } else
#endif
    {
        // By default, prefer vello > tessellation [w/ atlas] > raster atlas
        if (IsSupported(PathRendererStrategy::kComputeMSAA8, caps)) {
            fStrategy = PathRendererStrategy::kComputeMSAA8;
        } else if (caps->avoidMSAA()) {
            fStrategy = PathRendererStrategy::kRasterAtlas;
        } else if (caps->minPathSizeForMSAA() > 0) {
            fStrategy = PathRendererStrategy::kTessellationAndSmallAtlas;
        } else {
            fStrategy = PathRendererStrategy::kTessellation;
        }

        SkASSERT(IsSupported(fStrategy, caps));
    }

    const bool infinitySupport = caps->shaderCaps()->fInfinitySupport;
    const bool useStorageBuffers = caps->storageBufferSupport();
    const auto& bindingReq = caps->resourceBindingRequirements();
    auto layout = useStorageBuffers ? bindingReq.fStorageBufferLayout
                                    : bindingReq.fUniformBufferLayout;

    // Single-step renderers don't share RenderSteps
    auto initFromStep = [&](Renderer* renderer,
                            std::unique_ptr<RenderStep> singleStep,
                            DrawTypeFlags drawTypes) {
        std::string name = "SingleStep[";
        name += singleStep->name();
        name += "]";
        this->initRenderer(renderer, name, drawTypes, this->assumeOwnership(std::move(singleStep)));
    };

    // NOTE: We always initialize the tessellation RenderSteps because they are used for fallback
    // with all of the other atlas'ing path renderer strategies. We always initialize the
    // CoverageMaskRenderStep because it is used for mask filters even when the path renderer
    // strategy wouldn't use it to sample an atlas.
    initFromStep(&fConvexTessellatedWedges,
                 std::make_unique<TessellateWedgesRenderStep>(layout,
                        RenderStep::RenderStepID::kTessellateWedges_Convex,
                        infinitySupport, kDirectDepthLessPass, bufferManager),
                 DrawTypeFlags::kNonSimpleShape);
    initFromStep(&fTessellatedStrokes,
                 std::make_unique<TessellateStrokesRenderStep>(layout, infinitySupport),
                 DrawTypeFlags::kNonSimpleShape);
    initFromStep(&fCoverageMask,
                 std::make_unique<CoverageMaskRenderStep>(layout),
                 static_cast<DrawTypeFlags>((int) DrawTypeFlags::kNonSimpleShape |
                                            (int) InternalDrawTypeFlags::kCoverageMask));

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
        initFromStep(&fBitmapText[int(textVariant.fFormat)],
                     std::make_unique<BitmapTextRenderStep>(layout, textVariant.fFormat),
                     textVariant.fDrawType);
    }

    // SDF text (lcd and single channel)
    initFromStep(&fSDFText[/*lcd=*/true],
                 std::make_unique<SDFTextLCDRenderStep>(layout),
                 DrawTypeFlags::kSDFText_LCD);
    initFromStep(&fSDFText[/*lcd=*/false],
                 std::make_unique<SDFTextRenderStep>(layout),
                 DrawTypeFlags::kSDFText);

    initFromStep(&fAnalyticRRect,
                 std::make_unique<AnalyticRRectRenderStep>(layout, bufferManager),
                 DrawTypeFlags::kAnalyticRRect);
    initFromStep(&fPerEdgeAAQuad,
                 std::make_unique<PerEdgeAAQuadRenderStep>(layout, bufferManager),
                 DrawTypeFlags::kPerEdgeAAQuad);
    initFromStep(&fNonAABoundsFill,
                 std::make_unique<CoverBoundsRenderStep>(layout,
                        RenderStep::RenderStepID::kCoverBounds_NonAAFill,
                        kDirectDepthLessPass),
                 DrawTypeFlags::kNonAAFillRect);
    initFromStep(&fCircularArc,
                 std::make_unique<CircularArcRenderStep>(layout, bufferManager),
                 DrawTypeFlags::kCircularArc);
    initFromStep(&fAnalyticBlur,
                 std::make_unique<AnalyticBlurRenderStep>(layout),
                 DrawTypeFlags::kDropShadows);

    // vertices
    for (PrimitiveType primType : {PrimitiveType::kTriangles, PrimitiveType::kTriangleStrip}) {
        for (bool color : {false, true}) {
            for (bool texCoords : {false, true}) {
                DrawTypeFlags dtFlags = DrawTypeFlags::kDrawVertices;
                if (primType == PrimitiveType::kTriangles && color && !texCoords) {
                    // Android uses this drawVertices combination for drop shadows
                    dtFlags = static_cast<DrawTypeFlags>(dtFlags | DrawTypeFlags::kDropShadows);
                }

                int index = 4*(primType == PrimitiveType::kTriangleStrip) + 2*color + texCoords;
                initFromStep(&fVertices[index],
                             std::make_unique<VerticesRenderStep>(layout, primType, color,
                                                                  texCoords),
                             dtFlags);
            }
        }
    }

    // The tessellating path renderers that use stencil can share the cover steps.
    auto coverFill = std::make_unique<CoverBoundsRenderStep>(
            layout, RenderStep::RenderStepID::kCoverBounds_RegularCover, kRegularCoverPass);
    auto coverInverse = std::make_unique<CoverBoundsRenderStep>(
            layout, RenderStep::RenderStepID::kCoverBounds_InverseCover, kInverseCoverPass);

    for (bool evenOdd : {false, true}) {
        // These steps can be shared by regular and inverse fills
        auto stencilFan = std::make_unique<MiddleOutFanRenderStep>(layout, evenOdd);
        auto stencilCurve = std::make_unique<TessellateCurvesRenderStep>(
                layout, evenOdd, infinitySupport, bufferManager);
        auto stencilWedge =
                evenOdd ? std::make_unique<TessellateWedgesRenderStep>(layout,
                                RenderStep::RenderStepID::kTessellateWedges_EvenOdd,
                                infinitySupport, kEvenOddStencilPass, bufferManager)
                        : std::make_unique<TessellateWedgesRenderStep>(layout,
                                RenderStep::RenderStepID::kTessellateWedges_Winding,
                                infinitySupport, kWindingStencilPass, bufferManager);

        for (bool inverse : {false, true}) {
            static const char* kTessVariants[4] =
                    {"[winding]", "[evenodd]", "[inverse-winding]", "[inverse-evenodd]"};

            int index = 2*inverse + evenOdd; // matches SkPathFillType
            std::string variant = kTessVariants[index];

            const RenderStep* coverStep = inverse ? coverInverse.get() : coverFill.get();
            this->initRenderer(&fStencilTessellatedCurves[index],
                               "StencilTessellatedCurvesAndTris" + variant,
                               DrawTypeFlags::kNonSimpleShape,
                               stencilFan.get(),
                               stencilCurve.get(),
                               coverStep);

            this->initRenderer(&fStencilTessellatedWedges[index],
                               "StencilTessellatedWedges" + variant,
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

#ifdef SK_ENABLE_VELLO_SHADERS
    // Don't initialize Vello if the strategy wouldn't use it.
    if (fStrategy == PathRendererStrategy::kComputeAnalyticAA ||
        fStrategy == PathRendererStrategy::kComputeMSAA16 ||
        fStrategy == PathRendererStrategy::kComputeMSAA8) {
        fVelloRenderer = std::make_unique<VelloRenderer>(caps);
    }
#endif
}

} // namespace skgpu::graphite
