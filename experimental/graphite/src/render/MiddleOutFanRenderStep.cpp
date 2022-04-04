/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/render/MiddleOutFanRenderStep.h"

#include "experimental/graphite/src/DrawGeometry.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/render/StencilAndCoverDSS.h"

#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

namespace skgpu {

MiddleOutFanRenderStep::MiddleOutFanRenderStep(bool evenOdd)
        : RenderStep("MiddleOutFanRenderStep",
                     evenOdd ? "even-odd" : "winding",
                     Flags::kRequiresMSAA,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*vertexAttrs=*/{{"position",
                                       VertexAttribType::kFloat4,
                                       SkSLType::kFloat4}},
                     /*instanceAttrs=*/{}) {}

MiddleOutFanRenderStep::~MiddleOutFanRenderStep() {}

const char* MiddleOutFanRenderStep::vertexSkSL() const {
    return "     float4 devPosition = position;\n";
}

void MiddleOutFanRenderStep::writeVertices(DrawWriter* writer, const DrawGeometry& geom) const {
    // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
    // paths to SkPath just to iterate their pts/verbs
    SkPath path = geom.shape().asPath();

    const int maxCombinedFanEdges = MaxCombinedFanEdgesInPaths(path.countVerbs());
    const int maxTrianglesInFans = std::max(maxCombinedFanEdges - 2, 0);

    float depth = geom.order().depthAsFloat();

    DrawWriter::Vertices verts{*writer};
    verts.reserve(maxTrianglesInFans * 3);
    for (PathMiddleOutFanIter it(path); !it.done();) {
        for (auto [p0, p1, p2] : it.nextStack()) {
            // TODO: PathMiddleOutFanIter should use SkV2 instead of SkPoint?
            SkV2 p[3] = {{p0.fX, p0.fY}, {p1.fX, p1.fY}, {p2.fX, p2.fY}};
            SkV4 devPoints[3];
            geom.transform().mapPoints(p, devPoints, 3);

            verts.append(3) << devPoints[0].x << devPoints[0].y << depth << devPoints[0].w  // p0
                            << devPoints[1].x << devPoints[1].y << depth << devPoints[1].w  // p1
                            << devPoints[2].x << devPoints[2].y << depth << devPoints[2].w; // p2
        }
    }
}

void MiddleOutFanRenderStep::writeUniforms(const DrawGeometry&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu
