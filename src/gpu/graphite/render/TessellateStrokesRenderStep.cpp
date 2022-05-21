/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateStrokesRenderStep.h"

#include "src/gpu/graphite/DrawGeometry.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"


namespace skgpu::graphite {

namespace {

// TODO: De-duplicate with the same settings that's currently used for convex path rendering
// in StencilAndFillPathRenderer.cpp
static constexpr DepthStencilSettings kDirectShadingPass = {
        /*frontStencil=*/{},
        /*backStencil=*/ {},
        /*refValue=*/    0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

}  // namespace

// TODO: specify attributes and uniforms
TessellateStrokesRenderStep::TessellateStrokesRenderStep()
        : RenderStep("TessellateStrokeRenderStep",
                     "",
                     Flags::kRequiresMSAA,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectShadingPass,
                     /*vertexAttrs=*/  {},
                     /*instanceAttrs=*/{}) {}

TessellateStrokesRenderStep::~TessellateStrokesRenderStep() {}

const char* TessellateStrokesRenderStep::vertexSkSL() const {
    SkASSERT(false);
    return ""; // TODO: implement this
}

void TessellateStrokesRenderStep::writeVertices(DrawWriter* dw, const DrawGeometry& geom) const {
    // TODO: implement this

}

void TessellateStrokesRenderStep::writeUniforms(const DrawGeometry&,
                                                SkPipelineDataGatherer*) const {
    // TODO: implement this
}

}  // namespace skgpu::graphite
