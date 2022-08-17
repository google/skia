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
static constexpr Attribute ssboIndexAttribute = {"ssboIndex", VertexAttribType::kInt,
                                                 SkSLType::kInt};
static constexpr Attribute textureAttribute = {"texCoords", VertexAttribType::kFloat2,
                                               SkSLType::kFloat2};
// static constexpr Attribute colorAttribute = {"color", VertexAttribType::kFloat4,
//                                              SkSLType::kFloat4};

static constexpr std::initializer_list<Attribute> positionOnly = {positionAttribute,
                                                                  ssboIndexAttribute};
// static constexpr std::initializer_list<Attribute> withColor = {positionAttribute,
//                                                                colorAttribute};
static constexpr std::initializer_list<Attribute> withTexture = {positionAttribute,
                                                                 textureAttribute,
                                                                 ssboIndexAttribute};
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
                     Flags::kPerformsShading,
                     /*uniforms=*/{{"depth", SkSLType::kFloat}},
                     type,
                     kDirectShadingPass,
                     // TODO: Select attributes based upon whether the vertices has color and/or
                     // texture information.
                     /*vertexAttrs=*/hasTexture ? withTexture : positionOnly,
                     /*instanceAttrs=*/{})
                     , fHasColor(hasColor)
                     , fHasTexture(hasTexture) {
    // TODO: Remove the following line once fHasColor is used. Here for now to prevent a warning.
    (void) fHasColor;
}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
     // Add cases for color and color+texture.
    if (fHasTexture) {
        return R"(
            float4 devPosition = float4(position, 1.0, 1.0);
            stepLocalCoords = texCoords;
        )";
    } else {
        return R"(
            float4 devPosition = float4(position, 1.0, 1.0);
            stepLocalCoords = position;
        )";
    }
}

void VerticesRenderStep::writeVertices(DrawWriter* writer,
                                       const DrawParams& params,
                                       int ssboIndex) const {
    // TODO: Instead of fHasColor and fHasTexture, use info() and assert.
    // Add cases for color and color+texture.
    if (fHasTexture) {
        this->writeVerticesTexture(writer, params, ssboIndex);
    } else {
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
            verts.append(3) << devPoints[0].x << devPoints[0].y << ssboIndex
                            << devPoints[1].x << devPoints[1].y << ssboIndex
                            << devPoints[2].x << devPoints[2].y << ssboIndex;
        }
    }
}

void VerticesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  SkPipelineDataGatherer* gatherer) const {
    // Vertices are currently pre-transformed to device space on the CPU so no transformation matrix
    // is needed. Store PaintDepth as a uniform to avoid copying the same depth for each vertex.
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    gatherer->write(params.order().depthAsFloat());
}

void VerticesRenderStep::writeVerticesTexture(DrawWriter* writer, const DrawParams& params,
                                              int ssboIndex) const {
    SkVerticesPriv info(params.geometry().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkPoint* texCoords = info.texCoords();

    DrawWriter::Vertices verts{*writer};

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    while (vertProc(&state)) {
        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};

        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);

        verts.append(3) << devPoints[0].x << devPoints[0].y << texCoords[state.f0] << ssboIndex
                        << devPoints[1].x << devPoints[1].y << texCoords[state.f1] << ssboIndex
                        << devPoints[2].x << devPoints[2].y << texCoords[state.f2] << ssboIndex;
    }
}

}  // namespace skgpu::graphite
