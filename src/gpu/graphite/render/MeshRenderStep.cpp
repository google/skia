/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MeshRenderStep.h"

#include "src/gpu/graphite/PaintParamsKey.h"

#include "include/private/SkAssert.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVertState.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"

#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <cstdint>

namespace skgpu::graphite {

static SkVertices::VertexMode vertex_mode(SkMesh::Mode mode) {
    if (mode == SkMesh::Mode::kTriangles) {
        return SkVertices::VertexMode::kTriangles_VertexMode;
    } else {
        SkASSERT(mode == SkMesh::Mode::kTriangleStrip);
        return SkVertices::VertexMode::kTriangleStrip_VertexMode;
    }
}

MeshRenderStep::MeshRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStep::RenderStepID::kMesh,
                     Flags::kPerformsShading | Flags::kAppendVertices
                                             | Flags::kEmitsPrimitiveColor,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/{},
                     /*appendAttrs=*/{{"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}}) {}

MeshRenderStep::~MeshRenderStep() {}

std::string MeshRenderStep::vertexSkSL(const RootNodesInfo& roots) const {
    return "";
}

std::string MeshRenderStep::fragmentColorSkSL(const RootNodesInfo& roots) const {
    return "";
}

void MeshRenderStep::writeVertices(DrawWriter* writer,
                                   const DrawParams& params,
                                   uint32_t ssboIndex) const {
    const SkMesh& mesh = params.geometry().mesh();
    const SkMeshSpecification* spec = mesh.spec();

    SkASSERT(mesh.vertexBuffer()); // SkMesh::isValid() should catch this in Device::drawMesh().
    auto* cpuVertexBuffer = static_cast<const SkMeshPriv::CpuVertexBuffer*>(mesh.vertexBuffer());
    size_t vertexStride = spec->stride();
    size_t vertexCount = mesh.vertexCount();
    const uint8_t* vertexData = static_cast<const uint8_t*>(
                                    cpuVertexBuffer->peek()) + mesh.vertexOffset();

    const uint16_t* indices = nullptr;
    size_t indexCount = mesh.indexCount();
    if (mesh.indexBuffer()) {
        auto* cpuIndexBuffer = static_cast<const SkMeshPriv::CpuIndexBuffer*>(mesh.indexBuffer());
        indices = reinterpret_cast<const uint16_t*>(static_cast<const uint8_t*>(
                            cpuIndexBuffer->peek()) + mesh.indexOffset());
    }

    DrawWriter::Vertices verts(*writer);
    verts.reserve(indices ? indexCount : vertexCount);

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vertex_mode(mesh.mode()));
    while (vertProc(&state)) {
        VertexWriter vertWriter = verts.append(3);
        for (uint32_t i = 0; i < 3; ++i) {
            uint32_t vertIndex = i == 0 ? state.f0 : (i == 1 ? state.f1 : state.f2);
            const uint8_t* vertexDataBase = vertexData + vertIndex * vertexStride;

            for (const SkMeshSpecification::Attribute& attr : spec->attributes()) {
                vertWriter << VertexWriter::Array(
                                    vertexDataBase + attr.offset,
                                    SkMeshSpecificationPriv::AttrTypeByteSize(attr.type));
            }
            vertWriter << ssboIndex;
        }
    }
}

void MeshRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                              PipelineDataGatherer* gatherer) const {
    // TODO (nathanasanchez): Implement uniform writing.
}

}  // namespace skgpu::graphite
