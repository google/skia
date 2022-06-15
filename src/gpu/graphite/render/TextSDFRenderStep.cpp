/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TextSDFRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"

namespace skgpu::graphite {

namespace {
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

TextSDFRenderStep::TextSDFRenderStep(bool isA8)
        : RenderStep("TextSDFRenderStep",
                     isA8 ? "A8" : "565",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectShadingPass,
                     // TODO: set up attributes
                     /*vertexAttrs=*/  {},
                     /*instanceAttrs=*/{}) {
    // TODO: store if it's A8?
}

TextSDFRenderStep::~TextSDFRenderStep() {}

const char* TextSDFRenderStep::vertexSkSL() const {
    // TODO: write vertex shader
    return "float4 devPosition = float4(0, 0, 0, 1);\n";
}

void TextSDFRenderStep::writeVertices(DrawWriter* dw, const DrawParams& geom) const {
    // TODO: get this from the SubRun
}

void TextSDFRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // TODO
}

}  // namespace skgpu::graphite
