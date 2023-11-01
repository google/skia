/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MiddleOutFanRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

namespace skgpu::graphite {

MiddleOutFanRenderStep::MiddleOutFanRenderStep(bool evenOdd)
        : RenderStep("MiddleOutFanRenderStep",
                     evenOdd ? "even-odd" : "winding",
                     Flags::kRequiresMSAA,
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4}},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*vertexAttrs=*/
                            {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2}},
                     /*instanceAttrs=*/{}) {}

MiddleOutFanRenderStep::~MiddleOutFanRenderStep() {}

std::string MiddleOutFanRenderStep::vertexSkSL() const {
    return R"(
        float4 devPosition = localToDevice * float4(position, 0.0, 1.0);
        devPosition.z = depth;
        stepLocalCoords = position;
    )";
}

void MiddleOutFanRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::ushort2 ssboIndices) const {
    // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
    // paths to SkPath just to iterate their pts/verbs
    SkPath path = params.geometry().shape().asPath();

    const int maxTrianglesInFans = std::max(path.countVerbs() - 2, 0);

    float depth = params.order().depthAsFloat();

    DrawWriter::Vertices verts{*writer};
    verts.reserve(maxTrianglesInFans * 3);
    for (tess::PathMiddleOutFanIter it(path); !it.done();) {
        for (auto [p0, p1, p2] : it.nextStack()) {
            verts.append(3) << p0 << depth << ssboIndices
                            << p1 << depth << ssboIndices
                            << p2 << depth << ssboIndices;
        }
    }
}

void MiddleOutFanRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());
}

}  // namespace skgpu::graphite
