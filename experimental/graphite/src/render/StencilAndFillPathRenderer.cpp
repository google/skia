/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

#include "experimental/graphite/src/render/CoverBoundsRenderStep.h"
#include "experimental/graphite/src/render/MiddleOutFanRenderStep.h"
#include "experimental/graphite/src/render/TessellateCurvesRenderStep.h"
#include "include/core/SkPathTypes.h"

namespace skgpu {

const Renderer& Renderer::StencilAndFillPath(SkPathFillType fillType) {
    // Because each fill type uses a different stencil settings, there is one Renderer per type.
    // However, at each stage (stencil vs. cover), there are only two RenderSteps to branch on.
    static const MiddleOutFanRenderStep kWindingStencilFan{false};
    static const MiddleOutFanRenderStep kEvenOddStencilFan{true};
    static const TessellateCurvesRenderStep kWindingStencilCurves{false};
    static const TessellateCurvesRenderStep kEvenOddStencilCurves{true};
    static const CoverBoundsRenderStep kFill{false};
    static const CoverBoundsRenderStep kInverseFill{true};

    // TODO: Uncomment and include the curve stenciling steps to draw curved paths
    static const Renderer kWindingRenderer{"stencil-and-fill[winding]",
                                           &kWindingStencilFan,
                                           &kWindingStencilCurves,
                                           &kFill};
    static const Renderer kInverseWindingRenderer{"stencil-and-fill[inverse-winding]",
                                                  &kWindingStencilFan,
                                                  &kWindingStencilCurves,
                                                  &kInverseFill};
    static const Renderer kEvenOddRenderer{"stencil-and-fill[evenodd]",
                                           &kEvenOddStencilFan,
                                           &kEvenOddStencilCurves,
                                           &kFill};
    static const Renderer kInverseEvenOddRenderer{"stencil-and-fill[inverse-evenodd]",
                                                  &kEvenOddStencilFan,
                                                  &kEvenOddStencilCurves,
                                                  &kInverseFill};

    switch(fillType) {
        case SkPathFillType::kWinding: return kWindingRenderer;
        case SkPathFillType::kEvenOdd: return kEvenOddRenderer;
        case SkPathFillType::kInverseWinding: return kInverseWindingRenderer;
        case SkPathFillType::kInverseEvenOdd: return kInverseEvenOddRenderer;
    }
    SkUNREACHABLE;
}

} // namespace skgpu
