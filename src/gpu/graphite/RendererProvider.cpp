/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RendererProvider.h"

#include "include/core/SkPathTypes.h"
#include "include/core/SkVertices.h"
#include "src/gpu/graphite/render/BitmapTextRenderStep.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/CoverBoundsRenderStep.h"
#include "src/gpu/graphite/render/MiddleOutFanRenderStep.h"
#include "src/gpu/graphite/render/SDFTextRenderStep.h"
#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"
#include "src/gpu/graphite/render/TessellateStrokesRenderStep.h"
#include "src/gpu/graphite/render/TessellateWedgesRenderStep.h"
#include "src/gpu/graphite/render/VerticesRenderStep.h"

namespace skgpu::graphite {

RendererProvider::RendererProvider() {
    // This constructor requires all Renderers be densely packed so that it can simply iterate over
    // the fields directly and fill 'fRenderers' with every one that was initialized with a
    // non-empty renderer. While this is a little magical, it simplifies the rest of the logic
    // and can be enforced statically.
    static constexpr size_t kRendererSize = offsetof(RendererProvider, fRenderers) -
                                            offsetof(RendererProvider, fStencilTessellatedCurves);
    static_assert(kRendererSize % sizeof(Renderer) == 0, "Renderer declarations are not dense");

    // Single-step renderers don't share RenderSteps
    auto makeFromStep = [&](std::unique_ptr<RenderStep> singleStep) {
        std::string name = "SingleStep[";
        name += singleStep->name();
        name += "]";
        fRenderSteps.push_back(std::move(singleStep));
        return Renderer(name, fRenderSteps.back().get());
    };

    fConvexTessellatedWedges = makeFromStep(
            std::make_unique<TessellateWedgesRenderStep>("convex", kDirectDepthGreaterPass));
    fTessellatedStrokes = makeFromStep(std::make_unique<TessellateStrokesRenderStep>());
    for (bool a8 : {false, true}) {
        fBitmapText[a8] = makeFromStep(std::make_unique<BitmapTextRenderStep>(a8));
    }
    for (bool lcd : {false, true}) {
        fSDFText[lcd] = makeFromStep(std::make_unique<SDFTextRenderStep>(lcd));
    }
    for (PrimitiveType primType : {PrimitiveType::kTriangles, PrimitiveType::kTriangleStrip}) {
        for (bool color : {false, true}) {
            for (bool texCoords : {false, true}) {
                int index = 4*(primType == PrimitiveType::kTriangleStrip) + 2*color + texCoords;
                fVertices[index] = makeFromStep(
                        std::make_unique<VerticesRenderStep>(primType, color, texCoords));
            }
        }
    }

    // The tessellating path renderers that use stencil can share the cover steps.
    auto coverFill = std::make_unique<CoverBoundsRenderStep>(false);
    auto coverInverse = std::make_unique<CoverBoundsRenderStep>(true);

    for (bool evenOdd : {false, true}) {
        // These steps can be shared by regular and inverse fills
        auto stencilFan = std::make_unique<MiddleOutFanRenderStep>(evenOdd);
        auto stencilCurve = std::make_unique<TessellateCurvesRenderStep>(evenOdd);
        auto stencilWedge = evenOdd
                ? std::make_unique<TessellateWedgesRenderStep>("evenodd", kEvenOddStencilPass)
                : std::make_unique<TessellateWedgesRenderStep>("winding", kWindingStencilPass);

        for (bool inverse : {false, true}) {
            static const char* kTessVariants[4] =
                    {"[winding]", "[evenodd]", "[inverse-winding]", "[inverse-evenodd]"};

            int index = 2*inverse + evenOdd; // matches SkPathFillType
            std::string variant = kTessVariants[index];

            const RenderStep* coverStep = inverse ? coverInverse.get() : coverFill.get();
            fStencilTessellatedCurves[index] = Renderer("StencilTessellatedCurvesAndTris" + variant,
                                                        stencilFan.get(),
                                                        stencilCurve.get(),
                                                        coverStep);

            fStencilTessellatedWedges[index] = Renderer("StencilTessellatedWedges" + variant,
                                                        stencilWedge.get(),
                                                        coverStep);
        }

        fRenderSteps.push_back(std::move(stencilFan));
        fRenderSteps.push_back(std::move(stencilCurve));
        fRenderSteps.push_back(std::move(stencilWedge));
    }

    fRenderSteps.push_back(std::move(coverInverse));
    fRenderSteps.push_back(std::move(coverFill));

    // Fill out 'fRenderers' by iterating the "span" from fStencilTessellatedCurves to fRenderers
    // and checking if they've been skipped or not.
    SkSpan<Renderer> allRenderers = {fStencilTessellatedCurves, kRendererSize / sizeof(Renderer)};
    for (const Renderer& r : allRenderers) {
        if (r.numRenderSteps() > 0) {
            fRenderers.push_back(&r);
        }
    }
}

} // namespace skgpu::graphite
