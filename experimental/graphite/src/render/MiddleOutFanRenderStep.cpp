/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/render/MiddleOutFanRenderStep.h"

#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "experimental/graphite/src/render/StencilAndCoverDSS.h"

#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/PathTessellator.h"

namespace skgpu {

MiddleOutFanRenderStep::MiddleOutFanRenderStep(bool evenOdd)
        : RenderStep(Flags::kRequiresMSAA,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*vertexAttrs=*/{{"position",
                                       VertexAttribType::kFloat3,
                                       SkSLType::kFloat3}},
                     /*instanceAttrs=*/{}) {}

MiddleOutFanRenderStep::~MiddleOutFanRenderStep() {}

const char* MiddleOutFanRenderStep::vertexSkSL() const {
    return "     float4 devPosition = float4(position.xy, 0.0, position.z);\n";
}

void MiddleOutFanRenderStep::writeVertices(DrawWriter* writer,
                                           const SkIRect& bounds,
                                           const Transform& localToDevice,
                                           const Shape& shape) const {
    // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
    // paths to SkPath just to iterate their pts/verbs
    SkPath path = shape.asPath();

    const int maxCombinedFanEdges =
            PathTessellator::MaxCombinedFanEdgesInPathDrawList(path.countVerbs());
    const int maxTrianglesInFans = std::max(maxCombinedFanEdges - 2, 0);

    DrawWriter::Vertices verts{*writer};
    verts.reserve(maxTrianglesInFans * 3);
    for (PathMiddleOutFanIter it(path); !it.done();) {
        for (auto [p0, p1, p2] : it.nextStack()) {
            // TODO: PathMiddleOutFanIter should use SkV2 instead of SkPoint?
            SkV2 p[3] = {{p0.fX, p0.fY}, {p1.fX, p1.fY}, {p2.fX, p2.fY}};
            SkV4 devPoints[3];
            localToDevice.mapPoints(p, devPoints, 3);

            verts.append(3) << devPoints[0].x << devPoints[0].y << devPoints[0].w  // p0
                            << devPoints[1].x << devPoints[1].y << devPoints[1].w  // p1
                            << devPoints[2].x << devPoints[2].y << devPoints[2].w; // p2
        }
    }
}

sk_sp<SkUniformData> MiddleOutFanRenderStep::writeUniforms(Layout,
                                                           const SkIRect&,
                                                           const Transform&,
                                                           const Shape&) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
    return nullptr;
}

}  // namespace skgpu
