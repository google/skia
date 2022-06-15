/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateWedgesRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"

#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/MidpointContourParser.h"
#include "src/gpu/tessellate/PatchWriter.h"

namespace skgpu::graphite {

namespace {

using namespace skgpu::tess;

// Only kFanPoint, no stroke params, since this is for filled wedges.
// No explicit curve type, since we assume infinity is supported on GPUs using graphite
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
static constexpr PatchAttribs kAttribs = PatchAttribs::kFanPoint |
                                         PatchAttribs::kPaintDepth;

using Writer = PatchWriter<DynamicInstancesPatchAllocator<FixedCountWedges>,
                           Required<PatchAttribs::kFanPoint>,
                           Required<PatchAttribs::kPaintDepth>>;

}  // namespace

TessellateWedgesRenderStep::TessellateWedgesRenderStep(std::string_view variantName,
                                                       DepthStencilSettings depthStencilSettings)
        : RenderStep("TessellateWedgesRenderStep",
                     variantName,
                     Flags::kRequiresMSAA |
                     (depthStencilSettings.fDepthWriteEnabled ? Flags::kPerformsShading
                                                              : Flags::kNone),
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     depthStencilSettings,
                     /*vertexAttrs=*/  {{"resolveLevel_and_idx",
                                         VertexAttribType::kFloat2, SkSLType::kFloat2}},
                     /*instanceAttrs=*/{{"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
                                        {"fanPointAttrib", VertexAttribType::kFloat2,
                                                           SkSLType::kFloat2},
                                        {"depth", VertexAttribType::kFloat, SkSLType::kFloat}}) {
    SkASSERT(this->instanceStride() == PatchStride(kAttribs));
}

TessellateWedgesRenderStep::~TessellateWedgesRenderStep() {}

const char* TessellateWedgesRenderStep::vertexSkSL() const {
    return "float4 devPosition = float4("
               "tessellate_filled_wedge(resolveLevel_and_idx.x, resolveLevel_and_idx.y, p01, p23, "
                                        "fanPointAttrib), depth, 1.0);\n";
}

void TessellateWedgesRenderStep::writeVertices(DrawWriter* dw, const DrawParams& geom) const {
    SkPath path = geom.geometry().shape().asPath(); // TODO: Iterate the Shape directly

    BindBufferInfo fixedVertexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kVertex,
            FixedCountWedges::WriteVertexBuffer,
            FixedCountWedges::VertexBufferSize);
    BindBufferInfo fixedIndexBuffer = dw->bufferManager()->getStaticBuffer(
            BufferType::kIndex,
            FixedCountWedges::WriteIndexBuffer,
            FixedCountWedges::IndexBufferSize);

    int patchReserveCount = FixedCountWedges::PreallocCount(path.countVerbs());
    Writer writer{kAttribs, *dw, fixedVertexBuffer, fixedIndexBuffer, patchReserveCount};
    writer.updatePaintDepthAttrib(geom.order().depthAsFloat());

    // TODO: Is it better to pre-transform on the CPU and only have a matrix uniform to compute
    // local coords, or is it better to always transform on the GPU (less CPU usage, more
    // uniform data to upload, dependent on push constants or storage buffers for good batching)

    // Currently no additional transform is applied by the GPU.
    writer.setShaderTransform(wangs_formula::VectorXform{});
    // TODO: This doesn't handle perspective yet, and ideally wouldn't go through SkMatrix.
    // It may not be relevant, though, if transforms are applied on the GPU and we only need to
    // determine an approximate 2x2 for 'shaderXform' and Wang's formula evaluation.
    AffineMatrix m(geom.transform().matrix().asM33());

    // TODO: Essentially the same as PathWedgeTessellator::write_patches but with a different
    // PatchWriter template.
    // For wedges, we iterate over each contour explicitly, using a fan point position that is in
    // the midpoint of the current contour.
    MidpointContourParser parser{path};
    while (parser.parseNextContour()) {
        writer.updateFanPointAttrib(m.mapPoint(parser.currentMidpoint()));
        skvx::float2 lastPoint = {0, 0};
        skvx::float2 startPoint = {0, 0};
        for (auto [verb, pts, w] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove: {
                    startPoint = lastPoint = m.map1Point(pts);
                    break;
                }

                case SkPathVerb::kLine: {
                    // Unlike curve tessellation, wedges have to handle lines as part of the patch,
                    // effectively forming a single triangle with the fan point.
                    auto [p0, p1] = m.map2Points(pts);
                    writer.writeLine(p0, p1);
                    lastPoint = p1;
                    break;
                }

                case SkPathVerb::kQuad: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    writer.writeQuadratic(p0, p1, p2);
                    lastPoint = p2;
                    break;
                }

                case SkPathVerb::kConic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    writer.writeConic(p0, p1, p2, *w);
                    lastPoint = p2;
                    break;
                }

                case SkPathVerb::kCubic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto [p2, p3] = m.map2Points(pts+2);

                    writer.writeCubic(p0, p1, p2, p3);
                    lastPoint = p3;
                    break;
                }

                default: break;
            }
        }

        // Explicitly close the contour with another line segment, which also differs from curve
        // tessellation since that approach's triangle step automatically closes the contour.
        if (any(lastPoint != startPoint)) {
            writer.writeLine(lastPoint, startPoint);
        }
    }
}

void TessellateWedgesRenderStep::writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const {
    // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
}

}  // namespace skgpu::graphite
