/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/WideTileRenderStep.h"

#include "include/core/SkPath.h"
#include "include/private/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <algorithm>

namespace skgpu::graphite {

WideTileRenderStep::WideTileRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kWideTile,
                     Flags::kAppendInstances | Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthLessPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     {{"tileBounds", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*storageUniforms=*/{}) {}

WideTileRenderStep::~WideTileRenderStep() {}

std::string WideTileRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return "float2 position = float2(sk_VertexID < 2 ? tileBounds.x : tileBounds.z,"
                                    "(sk_VertexID % 2) == 0 ? tileBounds.y : tileBounds.w);"
           "float4 devPosition = float4(position, depth, 1.0);"
           "float3x3 invM = float3x3(mat0, mat1, mat2);"
           "float3 localCoords = invM * float3(position, 1.0);"
           "stepLocalCoords = localCoords.xy / localCoords.z;";
}

void WideTileRenderStep::writeVertices(DrawWriter* writer,
                                       StorageContext* /*storageContext*/,
                                       const DrawParams& params,
                                       uint32_t ssboIndex) const {
    const auto& tiles = params.geometry().wideTiles().tiles();
    if (tiles.empty()) {
        return;
    }

    SkM44 invM;
    if (!params.transform().matrix().invert(&invM)) {
        return;
    }

    DrawWriter::Instances instances{*writer, {}, {}, 4};
    instances.reserve(tiles.size());

    float depth = params.order().depthAsFloat();
    for (const auto& tile : tiles) {
        float l = static_cast<float>(tile.fX);
        float t = static_cast<float>(tile.fY);
        float r = static_cast<float>(tile.fX + tile.fWidth);
        float b = static_cast<float>(tile.fY + SparseStripConfig::kTileHeight);

        instances.append(1) << l << t << r << b << depth << ssboIndex
                            << invM.rc(0, 0) << invM.rc(1, 0) << invM.rc(3, 0)  // mat0
                            << invM.rc(0, 1) << invM.rc(1, 1) << invM.rc(3, 1)  // mat1
                            << invM.rc(0, 3) << invM.rc(1, 3) << invM.rc(3, 3); // mat2
    }
}

void WideTileRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                  PipelineDataGatherer* gatherer) const {}

}  // namespace skgpu::graphite
