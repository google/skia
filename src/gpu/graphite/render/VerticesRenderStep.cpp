/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkPipelineData.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

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

static constexpr Attribute positionAttribute = {"position", VertexAttribType::kFloat2,
                                                SkSLType::kFloat2};
// TODO: Create other vertex attributes for color and texture.
// static constexpr Attribute textureAttribute = {"texCoords", VertexAttribType::kUShort2,
//                                                SkSLType::kUShort2};
// static constexpr Attribute colorAttribute = {"color", VertexAttribType::kFloat4, SkSLType::kFloat4};

static constexpr std::initializer_list<Attribute> positionOnly = {positionAttribute};
// static constexpr std::initializer_list<Attribute> withColor = {positionAttribute, colorAttribute};
// static constexpr std::initializer_list<Attribute> withTexture = {positionAttribute,
//                                                                  textureAttribute};
// static constexpr std::initializer_list<Attribute> withTextureAndColor = {positionAttribute,
//                                                                          colorAttribute,
//                                                                          textureAttribute};

// static constexpr std::initializer_list<Attribute> attributeArray [4] = { positionOnly, withColor,
//                                                                          withTexture,
//                                                                          withTextureAndColor };
}  // namespace

VerticesRenderStep::VerticesRenderStep(PrimitiveType type, std::string_view variantName,
                                       bool hasColor, bool hasTexture)
        : RenderStep("VerticesRenderStep",
                     variantName,
                     Flags::kPerformsShading, //Add Flags::kRequiresMSAA?
                     /*uniforms=*/{{"depth", SkSLType::kFloat}},
                     type,
                     kDirectShadingPass,
                     // TODO: Select attributes based upon whether the vertices has color and/or
                     // texture information.
                     // hasTexture ? attributeArray[2] : attributeArray[hasColor + hasTexture],
                     /*vertexAttrs=*/positionOnly,
                     /*instanceAttrs=*/{})
                     , fHasColor(hasColor)
                     , fHasTexture(hasTexture) {}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    // TODO: Add SkSl for color and/or texture information
    if (fHasColor && !fHasTexture) {
        // return R"(
        //     float4 color = color;
        //     float4 devPosition = float4(position, depth, 1.0);\n)";
    } else if (fHasTexture && !fHasColor) {
        // return R"(
        //     int2 coords = int2(texCoords.x, texCoords.y);
        //     int texIdx = coords.x >> 13;
        //     float2 unormTexCoords = float2(coords.x & 0x1FFF, coords.y);

        //     float2 textureCoords = unormTexCoords * atlasSizeInv;
        //     float texIndex = float(texIdx);
        //     float4 devPosition = float4(position, depth, 1.0);\n)";
    } else if (fHasColor && fHasTexture) {
        // return R"(
        //     float4 color = color;
        //     float4 devPosition = float4(position, depth, 1.0);\n)";
    } else {
        // return "float4 devPosition = float4(position, depth, 1.0);\n";
    }
    return "float4 devPosition = float4(position, depth, 1.0);\n";
}

void VerticesRenderStep::writeVertices(DrawWriter* writer, const DrawParams& params) const {
    SkVerticesPriv info(params.geometry().vertices()->priv());

    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    DrawWriter::Vertices verts{*writer};
    verts.reserve(indexCount);

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    while (vertProc(&state)) {
        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};
        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);

        verts.append(3) << devPoints[0].x << devPoints[0].y
                        << devPoints[1].x << devPoints[1].y
                        << devPoints[2].x << devPoints[2].y;

      // TODO: Record color &/or texture information.
      // TODO: Assert fHasColors == info.hasColors() and fHasTexture == info.hasTexCoords().
      //  if (info.hasColors()) {
      //     const SkColor* colors = info.colors();
      //  }
      //  if (info.hasTexCoords()) {
      //     const SkPoint* texCoords = info.texCoords();
      //  }
    }
}

void VerticesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  SkPipelineDataGatherer* gatherer) const {
    // Vertices are currently pre-transformed to device space on the CPU so no transformation matrix
    // is needed. Store PaintDepth as a uniform to avoid copying the same depth for each vertex.
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    gatherer->write(params.order().depthAsFloat());
}

}  // namespace skgpu::graphite
