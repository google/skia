/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MiddleOutFanRenderStep.h"

#include "include/core/SkPath.h"
#include "include/private/base/SkDebug.h"
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
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

#include <algorithm>

namespace skgpu::graphite {

MiddleOutFanRenderStep::MiddleOutFanRenderStep(Layout layout, bool evenOdd)
        : RenderStep(layout,
                     evenOdd ? RenderStepID::kMiddleOutFan_EvenOdd
                             : RenderStepID::kMiddleOutFan_Winding,
                     Flags::kRequiresMSAA | Flags::kAppendVertices,
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4}},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                     {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                     {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}}}) {}

MiddleOutFanRenderStep::~MiddleOutFanRenderStep() {}

std::string MiddleOutFanRenderStep::vertexSkSL() const {
    return
        "float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n"
        "devPosition.z = depth;\n"
        "stepLocalCoords = position;\n";
}

void MiddleOutFanRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           uint32_t ssboIndex) const {
    // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
    // paths to SkPath just to iterate their pts/verbs
    SkPath path = params.geometry().shape().asPath();

    const int maxTrianglesInFans = std::max(path.countVerbs() - 2, 0);

    float depth = params.order().depthAsFloat();

    DrawWriter::Vertices verts{*writer};
    verts.reserve(maxTrianglesInFans * 3);
    for (tess::PathMiddleOutFanIter it(path); !it.done();) {
        for (auto [p0, p1, p2] : it.nextStack()) {
            verts.append(3) << p0 << depth << ssboIndex
                            << p1 << depth << ssboIndex
                            << p2 << depth << ssboIndex;
        }
    }
}

void MiddleOutFanRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());
}

}  // namespace skgpu::graphite
