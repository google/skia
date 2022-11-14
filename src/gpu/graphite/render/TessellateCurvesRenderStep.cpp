/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"

#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/PatchWriter.h"

namespace skgpu::graphite {

namespace {

using namespace skgpu::tess;

// No fan point or stroke params, since this is for filled curves (not strokes or wedges)
// No explicit curve type, since we assume infinity is supported on GPUs using graphite
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
static constexpr PatchAttribs kAttribs = PatchAttribs::kPaintDepth |
                                         PatchAttribs::kSsboIndex;
using Writer = PatchWriter<DynamicInstancesPatchAllocator<FixedCountCurves>,
                           Required<PatchAttribs::kPaintDepth>,
                           Required<PatchAttribs::kSsboIndex>,
                           AddTrianglesWhenChopping,
                           DiscardFlatCurves>;

}  // namespace

TessellateCurvesRenderStep::TessellateCurvesRenderStep(bool evenOdd)
        : RenderStep("TessellateCurvesRenderStep",
                     evenOdd ? "even-odd" : "winding",
                     Flags::kRequiresMSAA,
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4}},
                     PrimitiveType::kTriangles,
                     evenOdd ? kEvenOddStencilPass : kWindingStencilPass,
                     /*vertexAttrs=*/  {{"resolveLevel_and_idx",
                                         VertexAttribType::kFloat2, SkSLType::kFloat2}},
                     /*instanceAttrs=*/{{"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                                        {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt}}) {
    SkASSERT(this->instanceStride() == PatchStride(kAttribs));
}

TessellateCurvesRenderStep::~TessellateCurvesRenderStep() {}

const char* TessellateCurvesRenderStep::vertexSkSL() const {
    return R"(
        // TODO: Approximate perspective scaling to match how PatchWriter is configured
        // (or provide explicit tessellation level in instance data instead of replicating work).
        float2x2 vectorXform = float2x2(localToDevice[0].xy, localToDevice[1].xy);
        float2 localCoord = tessellate_filled_curve(
                vectorXform, resolveLevel_and_idx.x, resolveLevel_and_idx.y, p01, p23);
        float4 devPosition = localToDevice * float4(localCoord, 0.0, 1.0);
        devPosition.z = depth;
        stepLocalCoords = localCoord;
    )";
}

void TessellateCurvesRenderStep::writeVertices(DrawWriter* dw,
                                               const DrawParams& params,
                                               int ssboIndex) const {
    SkPath path = params.geometry().shape().asPath(); // TODO: Iterate the Shape directly

    BindBufferInfo fixedVertexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kVertex,
            FixedCountCurves::WriteVertexBuffer,
            FixedCountCurves::VertexBufferSize);
    BindBufferInfo fixedIndexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kIndex,
            FixedCountCurves::WriteIndexBuffer,
            FixedCountCurves::IndexBufferSize);

    int patchReserveCount = FixedCountCurves::PreallocCount(path.countVerbs());
    Writer writer{kAttribs, *dw, fixedVertexBuffer, fixedIndexBuffer, patchReserveCount};
    writer.updatePaintDepthAttrib(params.order().depthAsFloat());
    writer.updateSsboIndexAttrib(ssboIndex);

    // The vector xform approximates how the control points are transformed by the shader to
    // more accurately compute how many *parametric* segments are needed.
    // TODO: This doesn't account for perspective division yet, which will require updating the
    // approximate transform based on each verb's control points' bounding box.
    SkASSERT(params.transform().type() < Transform::Type::kProjection);
    writer.setShaderTransform(wangs_formula::VectorXform{params.transform().matrix()},
                              params.transform().maxScaleFactor());

    // TODO: For filled curves, the path verb loop is simple enough that it's not too big a deal
    // to copy the logic from PathCurveTessellator::write_patches. It may be required if we end
    // up switching to a shape iterator in graphite vs. a path iterator in ganesh, or if
    // graphite does not control point transformation on the CPU. On the  other hand, if we
    // provide a templated WritePatches function, the iterator could also be a template arg in
    // addition to PatchWriter's traits. Whatever pattern we choose will be based more on what's
    // best for the wedge and stroke case, which have more complex loops.
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kQuad:  writer.writeQuadratic(pts); break;
            case SkPathVerb::kConic: writer.writeConic(pts, *w); break;
            case SkPathVerb::kCubic: writer.writeCubic(pts);     break;
            default:                                             break;
        }
    }
}

void TessellateCurvesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                          PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());
}

}  // namespace skgpu::graphite
