/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkPipelineData.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
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

static constexpr Attribute kPositionAttr =
        {"position", VertexAttribType::kFloat2, SkSLType::kFloat2};
static constexpr Attribute kTexCoordAttr =
        {"texCoords", VertexAttribType::kFloat2, SkSLType::kFloat2};
static constexpr Attribute kColorAttr =
        {"vertColor", VertexAttribType::kUByte4_norm, SkSLType::kHalf4};
static constexpr Attribute kSsboIndexAttr =
        {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt};

static constexpr std::initializer_list<Attribute> kAttributes[4] = {
        /*positionOnly*/   {kPositionAttr, kSsboIndexAttr},
        /*color*/          {kPositionAttr, kColorAttr, kSsboIndexAttr},
        /*texCoords*/      {kPositionAttr, kTexCoordAttr, kSsboIndexAttr},
        /*color+texCoords*/{kPositionAttr, kColorAttr, kTexCoordAttr, kSsboIndexAttr}
    };

static constexpr std::initializer_list<Varying> kVaryings[2] = {
        /*none*/  {},
        /*color*/ {{"color", SkSLType::kHalf4}}
    };

std::string variant_name(PrimitiveType type, bool hasColor, bool hasTexCoords) {
    SkASSERT(type == PrimitiveType::kTriangles || type == PrimitiveType::kTriangleStrip);
    std::string name = (type == PrimitiveType::kTriangles ? "tris" : "tristrips");
    if (hasColor) {
        name += "-color";
    }
    if (hasTexCoords) {
        name += "-texCoords";
    }
    return name;
}

}  // namespace

VerticesRenderStep::VerticesRenderStep(PrimitiveType type, bool hasColor, bool hasTexCoords)
        : RenderStep("VerticesRenderStep",
                     variant_name(type, hasColor, hasTexCoords),
                     hasColor ? Flags::kEmitsPrimitiveColor | Flags::kPerformsShading
                              : Flags::kPerformsShading,
                     /*uniforms=*/{{"depth", SkSLType::kFloat}},
                     type,
                     kDirectShadingPass,
                     /*vertexAttrs=*/  kAttributes[2*hasTexCoords + hasColor],
                     /*instanceAttrs=*/{},
                     /*varyings=*/     kVaryings[hasColor])
        , fHasColor(hasColor)
        , fHasTexCoords(hasTexCoords) {}

VerticesRenderStep::~VerticesRenderStep() {}

const char* VerticesRenderStep::vertexSkSL() const {
    if (fHasColor && fHasTexCoords) {
        return R"(
            color = half4(vertColor.bgr * vertColor.a, vertColor.a);
            float4 devPosition = float4(position, 1.0, 1.0);
            stepLocalCoords = texCoords;
        )";
    } else if (fHasTexCoords) {
        return R"(
            float4 devPosition = float4(position, 1.0, 1.0);
            stepLocalCoords = texCoords;
        )";
    } else if (fHasColor) {
        return R"(
            color = half4(vertColor.bgr * vertColor.a, vertColor.a);
            float4 devPosition = float4(position, 1.0, 1.0);
            stepLocalCoords = position;
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
    if (fHasColor && fHasTexCoords) {
        writeVerticesColorAndTexture(writer, params, ssboIndex);
    } else if (fHasColor) {
        writeVerticesColor(writer, params, ssboIndex);
    } else if (fHasTexCoords) {
        writeVerticesTexture(writer, params, ssboIndex);
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

const char* VerticesRenderStep::fragmentColorSkSL() const {
    if (fHasColor) {
        return "primitiveColor = color;\n";
    } else {
        return "";
    }
}

void VerticesRenderStep::writeVerticesColorAndTexture(DrawWriter* writer, const DrawParams& params,
                                                      int ssboIndex) const {
    SkVerticesPriv info(params.geometry().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();
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

        verts.append(3) << devPoints[0].x << devPoints[0].y << colors[state.f0]
                        << texCoords[state.f0] << ssboIndex
                        << devPoints[1].x << devPoints[1].y << colors[state.f1]
                        << texCoords[state.f1] << ssboIndex
                        << devPoints[2].x << devPoints[2].y << colors[state.f2]
                        << texCoords[state.f2] << ssboIndex;

    }
}

void VerticesRenderStep::writeVerticesColor(DrawWriter* writer, const DrawParams& params,
                                            int ssboIndex) const {
    SkVerticesPriv info(params.geometry().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();

    DrawWriter::Vertices verts{*writer};
    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());

    while (vertProc(&state)) {
        SkV2 p[3] = {{positions[state.f0].x(), positions[state.f0].y()},
                     {positions[state.f1].x(), positions[state.f1].y()},
                     {positions[state.f2].x(), positions[state.f2].y()}};

        SkV4 devPoints[3];
        params.transform().mapPoints(p, devPoints, 3);

        verts.append(3) << devPoints[0].x << devPoints[0].y << colors[state.f0] << ssboIndex
                        << devPoints[1].x << devPoints[1].y << colors[state.f1] << ssboIndex
                        << devPoints[2].x << devPoints[2].y << colors[state.f2] << ssboIndex;
    }
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
