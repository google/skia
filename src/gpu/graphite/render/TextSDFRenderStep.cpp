/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TextSDFRenderStep.h"

#include "src/core/SkPipelineData.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/text/gpu/SubRunContainer.h"

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
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2},
                                   {"distanceAdjust", SkSLType::kFloat}},
                     PrimitiveType::kTriangles,
                     kDirectShadingPass,
                     /*vertexAttrs=*/
                     {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"texCoords", VertexAttribType::kUShort2, SkSLType::kUShort2}},
                     /*instanceAttrs=*/{}) {
    // TODO: store if it's A8?
}

TextSDFRenderStep::~TextSDFRenderStep() {}

const char* TextSDFRenderStep::vertexSkSL() const {
    return R"(
        int2 coords = int2(texCoords.x, texCoords.y);
        int texIdx = coords.x >> 13;

        // TODO: these should be varyings
        float2 unormTexCoords = float2(coords.x & 0x1FFF, coords.y);
        float2 textureCoords = unormTexCoords * atlasSizeInv;
        float texIndex = float(texIdx);

        float4 devPosition = float4(position, depth, 1);
        )";
}

void TextSDFRenderStep::writeVertices(DrawWriter* dw, const DrawParams& params) const {
    const SubRunData& subRunData = params.geometry().subRunData();
    subRunData.subRun()->fillVertexData(dw, subRunData.startGlyphIndex(), subRunData.glyphCount(),
                                        params.order().depthAsFloat(), params.transform());
}

void TextSDFRenderStep::writeUniforms(const DrawParams& params,
                                      SkPipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    // TODO: get this from the actual texture size via the SubRunData
    skvx::float2 atlasDimensionsInverse = {1.f/1024, 1.f/1024};
    gatherer->write(atlasDimensionsInverse);

    // TODO: get this from DistanceFieldAdjustTable and luminance color (set in SubRunData?)
    float gammaCorrection = 0.1f;
    gatherer->write(gammaCorrection);
}

}  // namespace skgpu::graphite
