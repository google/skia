/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"

#include "src/gpu/tessellate/AffineMatrix.h"
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
                     /*uniforms=*/{},
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
    return "float4 devPosition = float4("
               "tessellate_filled_curve(resolveLevel_and_idx.x, resolveLevel_and_idx.y, p01, p23), "
               "depth, 1.0);\n";
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

    // TODO: Is it better to pre-transform on the CPU and only have a matrix uniform to compute
    // local coords, or is it better to always transform on the GPU (less CPU usage, more
    // uniform data to upload, dependent on push constants or storage buffers for good batching)

    // Currently no additional transform is applied by the GPU.
    writer.setShaderTransform(wangs_formula::VectorXform{});
    // TODO: This doesn't handle perspective yet, and ideally wouldn't go through SkMatrix.
    // It may not be relevant, though, if transforms are applied on the GPU and we only need to
    // determine an approximate 2x2 for 'shaderXform' and Wang's formula evaluation.
    AffineMatrix m(params.transform().matrix().asM33());

    // TODO: For filled curves, the path verb loop is simple enough that it's not too big a deal
    // to copy the logic from PathCurveTessellator::write_patches. It may be required if we end
    // up switching to a shape iterator in graphite vs. a path iterator in ganesh, or if
    // graphite does not control point transformation on the CPU. On the  other hand, if we
    // provide a templated WritePatches function, the iterator could also be a template arg in
    // addition to PatchWriter's traits. Whatever pattern we choose will be based more on what's
    // best for the wedge and stroke case, which have more complex loops.
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kQuad: {
                auto [p0, p1] = m.map2Points(pts);
                auto p2 = m.map1Point(pts+2);

                writer.writeQuadratic(p0, p1, p2);
                break;
            }

            case SkPathVerb::kConic: {
                auto [p0, p1] = m.map2Points(pts);
                auto p2 = m.map1Point(pts+2);

                writer.writeConic(p0, p1, p2, *w);
                break;
            }

            case SkPathVerb::kCubic: {
                auto [p0, p1] = m.map2Points(pts);
                auto [p2, p3] = m.map2Points(pts+2);

                writer.writeCubic(p0, p1, p2, p3);
                break;
            }

            default: break;
        }
    }
}

void TessellateCurvesRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                          SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
