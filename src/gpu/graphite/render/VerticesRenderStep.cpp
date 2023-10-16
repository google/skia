/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

namespace {

static constexpr Attribute kPositionAttr =
        {"position", VertexAttribType::kFloat2, SkSLType::kFloat2};
static constexpr Attribute kTexCoordAttr =
        {"texCoords", VertexAttribType::kFloat2, SkSLType::kFloat2};
static constexpr Attribute kColorAttr =
        {"vertColor", VertexAttribType::kUByte4_norm, SkSLType::kHalf4};
static constexpr Attribute kSsboIndexAttr =
        {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt};

static constexpr Attribute kAttributePositionOnly[] =
        {kPositionAttr, kSsboIndexAttr};
static constexpr Attribute kAttributeColor[] =
        {kPositionAttr, kColorAttr, kSsboIndexAttr};
static constexpr Attribute kAttributeTexCoords[] =
        {kPositionAttr, kTexCoordAttr, kSsboIndexAttr};
static constexpr Attribute kAttributeColorAndTexCoords[] =
        {kPositionAttr, kColorAttr, kTexCoordAttr, kSsboIndexAttr};

static constexpr SkSpan<const Attribute> kAttributes[4] = {
        kAttributePositionOnly,
        kAttributeColor,
        kAttributeTexCoords,
        kAttributeColorAndTexCoords,
    };

static constexpr Varying kVaryingColor[] =
        {{"color", SkSLType::kHalf4}};

static constexpr SkSpan<const Varying> kVaryings[2] = {
        /*none*/  {},
        /*color*/ kVaryingColor
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
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4},
                                   {"depth", SkSLType::kFloat}},
                     type,
                     kDirectDepthGEqualPass,
                     /*vertexAttrs=*/  kAttributes[2*hasTexCoords + hasColor],
                     /*instanceAttrs=*/{},
                     /*varyings=*/     kVaryings[hasColor])
        , fHasColor(hasColor)
        , fHasTexCoords(hasTexCoords) {}

VerticesRenderStep::~VerticesRenderStep() {}

std::string VerticesRenderStep::vertexSkSL() const {
    if (fHasColor && fHasTexCoords) {
        return R"(
            color = half4(vertColor.bgr * vertColor.a, vertColor.a);
            float4 devPosition = localToDevice * float4(position, 0.0, 1.0);
            devPosition.z = depth;
            stepLocalCoords = texCoords;
        )";
    } else if (fHasTexCoords) {
        return R"(
            float4 devPosition = localToDevice * float4(position, 0.0, 1.0);
            devPosition.z = depth;
            stepLocalCoords = texCoords;
        )";
    } else if (fHasColor) {
        return R"(
            color = half4(vertColor.bgr * vertColor.a, vertColor.a);
            float4 devPosition = localToDevice * float4(position, 0.0, 1.0);
            devPosition.z = depth;
            stepLocalCoords = position;
        )";
    } else {
        return R"(
            float4 devPosition = localToDevice * float4(position, 0.0, 1.0);
            devPosition.z = depth;
            stepLocalCoords = position;
        )";
    }
}

const char* VerticesRenderStep::fragmentColorSkSL() const {
    if (fHasColor) {
        return "primitiveColor = color;\n";
    } else {
        return "";
    }
}

void VerticesRenderStep::writeVertices(DrawWriter* writer,
                                       const DrawParams& params,
                                       int ssboIndex) const {
    SkVerticesPriv info(params.geometry().vertices()->priv());
    const int vertexCount = info.vertexCount();
    const int indexCount = info.indexCount();
    const SkPoint* positions = info.positions();
    const uint16_t* indices = info.indices();
    const SkColor* colors = info.colors();
    const SkPoint* texCoords = info.texCoords();

    // This should always be the case if the Renderer was chosen appropriately, but the vertex
    // writing loop is set up in such a way that if the shader expects color or tex coords and they
    // are missing, it will just read 0s, so release builds are safe.
    SkASSERT(fHasColor == SkToBool(colors));
    SkASSERT(fHasTexCoords == SkToBool(texCoords));

    // TODO: We could access the writer's DrawBufferManager and upload the SkVertices index buffer
    // but that would require we manually manage the VertexWriter for interleaving the position,
    // color, and tex coord arrays together. This wouldn't be so bad if we let ::Vertices() take
    // a CPU index buffer that indexes into the accumulated vertex data (and handles offsetting for
    // merged drawIndexed calls), or if we could bind multiple attribute sources and copy the
    // position/color/texCoord data separately in bulk w/o using an Appender.
    DrawWriter::Vertices verts{*writer};
    verts.reserve(indices ? indexCount : vertexCount);

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(info.mode());
    while (vertProc(&state)) {
        verts.append(3) << positions[state.f0]
                        << VertexWriter::If(fHasColor, colors ? colors[state.f0]
                                                              : SK_ColorTRANSPARENT)
                        << VertexWriter::If(fHasTexCoords, texCoords ? texCoords[state.f0]
                                                                     : SkPoint{0.f, 0.f})
                        << ssboIndex
                        << positions[state.f1]
                        << VertexWriter::If(fHasColor, colors ? colors[state.f1]
                                                              : SK_ColorTRANSPARENT)
                        << VertexWriter::If(fHasTexCoords, texCoords ? texCoords[state.f1]
                                                                     : SkPoint{0.f, 0.f})
                        << ssboIndex
                        << positions[state.f2]
                        << VertexWriter::If(fHasColor, colors ? colors[state.f2]
                                                              : SK_ColorTRANSPARENT)
                        << VertexWriter::If(fHasTexCoords, texCoords ? texCoords[state.f2]
                                                                     : SkPoint{0.f, 0.f})
                        << ssboIndex;
    }
}

void VerticesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  PipelineDataGatherer* gatherer) const {
    // Vertices are transformed on the GPU. Store PaintDepth as a uniform to avoid copying the
    // same depth for each vertex.
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    gatherer->write(params.transform().matrix());
    gatherer->write(params.order().depthAsFloat());
}

}  // namespace skgpu::graphite
