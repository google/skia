/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TextDirectRenderStep.h"

#include "src/core/SkPipelineData.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/text/gpu/SubRunContainer.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

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

// TODO: These attributes encompass only the most basic shaders.
// Do we need wide color? Do we need float3 positions? Do we need to handle float texcoords?
static const std::initializer_list<Attribute> kMaskAttrs =
        {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
         {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
         {"color", VertexAttribType::kUByte4_norm, SkSLType::kFloat2},
         {"texCoords", VertexAttribType::kUShort2, SkSLType::kUShort2}};
static const std::initializer_list<Attribute> kBitmapAttrs =
        {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
         {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
         {"texCoords", VertexAttribType::kUShort2, SkSLType::kUShort2}};

TextDirectRenderStep::TextDirectRenderStep(bool isMask)
        : RenderStep("TextDirectRenderStep",
                     isMask ? "mask" : "bitmap",
                     Flags::kPerformsShading,
                     /*uniforms=*/{{"atlasSizeInv", SkSLType::kFloat2}},
                     PrimitiveType::kTriangles,
                     kDirectShadingPass,
                     /*vertexAttrs=*/ isMask ? kMaskAttrs : kBitmapAttrs,
                     /*instanceAttrs=*/{})
        , fIsMask(isMask) {}

TextDirectRenderStep::~TextDirectRenderStep() {}

const char* TextDirectRenderStep::vertexSkSL() const {
    // TODO: switch shader text depending on if it's a mask or not (need to include color)
    return R"(
        int2 coords = int2(texCoords.x, texCoords.y);
        int texIdx = coords.x >> 13;
        float2 unormTexCoords = float2(coords.x & 0x1FFF, coords.y);

        // TODO: these should be varyings
        float2 textureCoords = unormTexCoords * atlasSizeInv;
        float texIndex = float(texIdx);
        // TODO: if there's color, it should pass through a varying as well

        float4 devPosition = float4(position, depth, 1);
        )";
}

void TextDirectRenderStep::writeVertices(DrawWriter* dw, const DrawParams& params) const {
    const SubRunData& subRunData = params.geometry().subRunData();
    // TODO: pass through the color from the SkPaint via the SubRunData
    subRunData.subRun()->fillVertexData(dw, subRunData.startGlyphIndex(), subRunData.glyphCount(),
                                        SK_ColorBLACK, params.order().depthAsFloat(),
                                        params.transform());
}

void TextDirectRenderStep::writeUniforms(const DrawParams& params,
                                         SkPipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    // TODO: get this from the actual texture size via the SubRunData
    skvx::float2 atlasDimensionsInverse = {1.f/1024, 1.f/1024};
    gatherer->write(atlasDimensionsInverse);
}

}  // namespace skgpu::graphite
