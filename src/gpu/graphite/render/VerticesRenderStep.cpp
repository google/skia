/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/VerticesRenderStep.h"

#include "include/core/SkColor.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <cstdint>

namespace skgpu::graphite {

namespace {

static constexpr Attribute kPositionAttr =
        {"position", VertexAttribType::kFloat2, SkSLType::kFloat2};
static constexpr Attribute kTexCoordAttr =
        {"texCoords", VertexAttribType::kFloat2, SkSLType::kFloat2};
static constexpr Attribute kColorAttr =
        {"vertColor", VertexAttribType::kUByte4_norm, SkSLType::kHalf4};
static constexpr Attribute kSsboIndexAttr =
        {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2};

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

RenderStep::RenderStepID variant_id(PrimitiveType type, bool hasColor, bool hasTexCoords) {
    if (type == PrimitiveType::kTriangles) {
        if (hasColor) {
            if (hasTexCoords) {
                return RenderStep::RenderStepID::kVertices_TrisColorTexCoords;
            } else {
                return RenderStep::RenderStepID::kVertices_TrisColor;
            }
        } else {
            if (hasTexCoords) {
                return RenderStep::RenderStepID::kVertices_TrisTexCoords;
            } else {
                return RenderStep::RenderStepID::kVertices_Tris;
            }
        }
    } else {
        SkASSERT(type == PrimitiveType::kTriangleStrip);

        if (hasColor) {
            if (hasTexCoords) {
                return RenderStep::RenderStepID::kVertices_TristripsColorTexCoords;
            } else {
                return RenderStep::RenderStepID::kVertices_TristripsColor;
            }
        } else {
            if (hasTexCoords) {
                return RenderStep::RenderStepID::kVertices_TristripsTexCoords;
            } else {
                return RenderStep::RenderStepID::kVertices_Tristrips;
            }
        }
    }
}

}  // namespace

VerticesRenderStep::VerticesRenderStep(PrimitiveType type, bool hasColor, bool hasTexCoords)
        : RenderStep(variant_id(type, hasColor, hasTexCoords),
                     (hasColor ? Flags::kEmitsPrimitiveColor : Flags::kNone) |
                     Flags::kPerformsShading | Flags::kAppendVertices,
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4},
                                   {"depth", SkSLType::kFloat}},
                     type,
                     kDirectDepthGEqualPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/kAttributes[2*hasTexCoords + hasColor],
                     /*varyings=*/   kVaryings[hasColor])
        , fHasColor(hasColor)
        , fHasTexCoords(hasTexCoords) {}

VerticesRenderStep::~VerticesRenderStep() {}

std::string VerticesRenderStep::vertexSkSL() const {
    if (fHasColor && fHasTexCoords) {
        return
            "color = half4(vertColor.bgr * vertColor.a, vertColor.a);\n"
            "float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n"
            "devPosition.z = depth;\n"
            "stepLocalCoords = texCoords;\n";
    } else if (fHasTexCoords) {
        return
            "float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n"
            "devPosition.z = depth;\n"
            "stepLocalCoords = texCoords;\n";
    } else if (fHasColor) {
        return
            "color = half4(vertColor.bgr * vertColor.a, vertColor.a);\n"
            "float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n"
            "devPosition.z = depth;\n"
            "stepLocalCoords = position;\n";
    } else {
        return
            "float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n"
            "devPosition.z = depth;\n"
            "stepLocalCoords = position;\n";
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
                                       skvx::uint2 ssboIndices) const {
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
                        << ssboIndices
                        << positions[state.f1]
                        << VertexWriter::If(fHasColor, colors ? colors[state.f1]
                                                              : SK_ColorTRANSPARENT)
                        << VertexWriter::If(fHasTexCoords, texCoords ? texCoords[state.f1]
                                                                     : SkPoint{0.f, 0.f})
                        << ssboIndices
                        << positions[state.f2]
                        << VertexWriter::If(fHasColor, colors ? colors[state.f2]
                                                              : SK_ColorTRANSPARENT)
                        << VertexWriter::If(fHasTexCoords, texCoords ? texCoords[state.f2]
                                                                     : SkPoint{0.f, 0.f})
                        << ssboIndices;
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
